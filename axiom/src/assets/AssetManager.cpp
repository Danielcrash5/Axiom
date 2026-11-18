/**
 * @file axiom/src/assets/AssetManager.cpp
 * @brief Implements the asset manager module.
 */

#include <axiom/assets/AssetManager.h>
#include <iostream>

namespace axiom {

    void AssetManager::init(size_t workerThreadCount) {
        if (sInitialized) {
            return;
        }

        sRegistry.init();
        if (workerThreadCount > 0) {
            sWorkerPool = std::make_unique<WorkerPool>(workerThreadCount);
        } else {
            sWorkerPool.reset();
        }
        sInitialized = true;
    }

    void AssetManager::shutdown() {
        if (!sInitialized) {
            return;
        }

        sWorkerPool.reset(); // Destruktor joint alle Worker-Threads
        std::lock_guard lock(sCacheMutex);
        sCache.clear();
        sLoaders.clear();
        sRegistry.shutdown();
        sCacheHits = 0;
        sTotalBytesLoaded = 0;
        sInitialized = false;
    }

    std::shared_ptr<AssetControlBlock> AssetManager::getOrCreateControlBlock(TypedUUID uuid) {
        std::lock_guard lock(sCacheMutex);
        auto it = sCache.find(uuid);
        if (it != sCache.end()) {
            if (auto controlBlock = it->second.controlBlock.lock()) {
                return controlBlock;
            }
            sCache.erase(it);
        }

        IAssetLoader *loader = getLoader(uuid.type);
        auto deleter = [uuid, loader](AssetControlBlock *controlBlock) {
            if (controlBlock->data && loader) {
                loader->unload(controlBlock->data);
                controlBlock->data = nullptr;
            }

            {
                std::lock_guard cacheLock(sCacheMutex);
                if (sTotalBytesLoaded >= controlBlock->sizeBytes) {
                    sTotalBytesLoaded -= controlBlock->sizeBytes;
                }
                auto cacheIt = sCache.find(uuid);
                if (cacheIt != sCache.end()) {
                    sCache.erase(cacheIt);
                }
            }

            delete controlBlock;
        };

        std::shared_ptr<AssetControlBlock> controlBlock(new AssetControlBlock, deleter);
        CacheEntry entry{controlBlock, uuid, uuid.type, nullptr};
        sCache[uuid] = entry;
        return controlBlock;
    }

    IAssetLoader *AssetManager::getLoader(AssetTypeId typeId) {
        auto it = sLoaders.find(typeId);
        if (it != sLoaders.end()) {
            return it->second;
        }

        std::cerr << "AssetManager: Kein Loader für Typ " << static_cast<uint32_t>(typeId)
                  << " registriert\n";
        return nullptr;
    }

    void AssetManager::registerLoader(AssetTypeId typeId, IAssetLoader *loader) {
        if (!loader) {
            return;
        }
        sLoaders[typeId] = loader;
    }

    void AssetManager::loadAsyncInternal(TypedUUID uuid, const AssetRegistryEntry &entry) {
        std::shared_ptr<AssetControlBlock> controlBlock;
        {
            std::lock_guard lock(sCacheMutex);
            auto it = sCache.find(uuid);
            if (it == sCache.end()) {
                return;
            }
            controlBlock = it->second.controlBlock.lock();
        }
        if (!controlBlock) {
            return;
        }

        // Lade die Datei
        std::vector<uint8_t> data;
        if (!VFS::readFile(entry.physicalPath, data)) {
            std::cerr << "AssetManager: Fehler beim Lesen von " << entry.physicalPath
                      << "\n";
            controlBlock->state.store(AssetLoadState::failed, std::memory_order_release);
            return;
        }

        // Hole den Loader
        IAssetLoader *loader = getLoader(entry.uuid.type);
        if (!loader) {
            controlBlock->state.store(AssetLoadState::failed, std::memory_order_release);
            return;
        }

        // Dekodiere das Asset
        controlBlock->state.store(AssetLoadState::loading, std::memory_order_release);
        size_t sizeBytes = 0;
        void *assetData = loader->load(data, sizeBytes);
        if (!assetData) {
            std::cerr << "AssetManager: Fehler beim Decoding von " << entry.physicalPath
                      << "\n";
            controlBlock->state.store(AssetLoadState::failed, std::memory_order_release);
            return;
        }

        // Speichere die Daten und mark als ready
        controlBlock->data = assetData;
        controlBlock->sizeBytes = sizeBytes;
        controlBlock->sizeBytes = sizeBytes;
        sTotalBytesLoaded += sizeBytes;
        controlBlock->state.store(AssetLoadState::loaded, std::memory_order_release);
    }

    void AssetManager::resolveDependencies(const TypedUUID &uuid,
                                            std::vector<TypedUUID> &outQueue) {
        const auto &deps = sRegistry.getDependencies(uuid);
        for (const auto &dep : deps) {
            if (dep.depType == AssetDependencyType::hard) {
                outQueue.push_back(dep.targetUUID);
            }
        }
    }

    void AssetManager::loadDependenciesInternal(
        const TypedUUID &uuid, std::vector<std::shared_ptr<AssetControlBlock>> &held,
        std::unordered_set<TypedUUID> &visited) {
        for (const auto &dependency : sRegistry.getDependencies(uuid)) {
            if (dependency.depType != AssetDependencyType::hard ||
                !visited.insert(dependency.targetUUID).second) {
                continue;
            }

            const AssetRegistryEntry *entry = sRegistry.get(dependency.targetUUID);
            if (!entry) {
                continue;
            }

            auto controlBlock = getOrCreateControlBlock(dependency.targetUUID);
            AssetLoadState expected = AssetLoadState::unloaded;
            if (controlBlock->state.compare_exchange_strong(expected,
                                                             AssetLoadState::queued)) {
                loadAsyncInternal(dependency.targetUUID, *entry);
            } else {
                while (controlBlock->state.load(std::memory_order_acquire) ==
                       AssetLoadState::queued ||
                       controlBlock->state.load(std::memory_order_acquire) ==
                           AssetLoadState::loading) {
                    std::this_thread::yield();
                }
            }

            if (controlBlock->state.load(std::memory_order_acquire) ==
                AssetLoadState::loaded) {
                held.push_back(controlBlock);
                loadDependenciesInternal(dependency.targetUUID, held, visited);
            }
        }
    }

    size_t AssetManager::getLoadedAssetCount() {
        std::lock_guard lock(sCacheMutex);
        size_t count = 0;
        for (const auto &[uuid, entry] : sCache) {
            auto controlBlock = entry.controlBlock.lock();
            if (controlBlock &&
                controlBlock->state.load(std::memory_order_acquire) ==
                AssetLoadState::loaded) {
                count++;
            }
        }
        return count;
    }

    size_t AssetManager::getTotalMemoryUsage() {
        return sTotalBytesLoaded;
        // TODO: Bessere Statistik-Tracking pro Asset
    }

    size_t AssetManager::getCacheHitCount() {
        return sCacheHits;
    }

} // namespace axiom
