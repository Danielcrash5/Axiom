#include <axiom/assets/AssetManager.h>
#include <iostream>

namespace axiom {

    void AssetManager::Init(size_t workerThreadCount) {
        s_Registry.Init();
        s_WorkerPool = std::make_unique<WorkerPool>(workerThreadCount);
    }

    void AssetManager::Shutdown() {
        s_WorkerPool.reset(); // Destruktor joint alle Worker-Threads
        std::lock_guard lock(s_CacheMutex);
        s_Cache.clear();
        s_Loaders.clear();
        s_Registry.Shutdown();
    }

    std::shared_ptr<AssetControlBlock> AssetManager::GetOrCreateControlBlock(TypedUUID uuid) {
        std::lock_guard lock(s_CacheMutex);
        auto it = s_Cache.find(uuid);
        if (it != s_Cache.end()) {
            return it->second.controlBlock;
        }

        auto controlBlock = std::make_shared<AssetControlBlock>();
        CacheEntry entry{controlBlock, uuid, uuid.type, nullptr};
        s_Cache[uuid] = entry;
        return controlBlock;
    }

    IAssetLoader *AssetManager::GetLoader(AssetTypeId typeId) {
        auto it = s_Loaders.find(typeId);
        if (it != s_Loaders.end()) {
            return it->second;
        }

        std::cerr << "AssetManager: Kein Loader für Typ " << static_cast<uint32_t>(typeId)
                  << " registriert\n";
        return nullptr;
    }

    void AssetManager::RegisterLoader(AssetTypeId typeId, IAssetLoader *loader) {
        if (!loader) {
            return;
        }
        s_Loaders[typeId] = loader;
    }

    void AssetManager::LoadAsync_Internal(TypedUUID uuid, const AssetRegistryEntry &entry) {
        auto it = s_Cache.find(uuid);
        if (it == s_Cache.end()) {
            // Cache-Eintrag sollte existieren (wurde GetOrCreateControlBlock aufgerufen)
            return;
        }

        auto &cacheEntry = it->second;
        auto controlBlock = cacheEntry.controlBlock;

        // Lade die Datei
        std::vector<uint8_t> data;
        if (!VFS::ReadFile(entry.physicalPath, data)) {
            std::cerr << "AssetManager: Fehler beim Lesen von " << entry.physicalPath
                      << "\n";
            controlBlock->state.store(AssetLoadState::Failed, std::memory_order_release);
            return;
        }

        // Hole den Loader
        IAssetLoader *loader = GetLoader(entry.uuid.type);
        if (!loader) {
            controlBlock->state.store(AssetLoadState::Failed, std::memory_order_release);
            return;
        }

        // Dekodiere das Asset
        controlBlock->state.store(AssetLoadState::Loading, std::memory_order_release);
        size_t sizeBytes = 0;
        void *assetData = loader->Load(data, sizeBytes);
        if (!assetData) {
            std::cerr << "AssetManager: Fehler beim Decoding von " << entry.physicalPath
                      << "\n";
            controlBlock->state.store(AssetLoadState::Failed, std::memory_order_release);
            return;
        }

        // Speichere die Daten und mark als ready
        controlBlock->data = assetData;
        s_TotalBytesLoaded += sizeBytes;
        controlBlock->state.store(AssetLoadState::Loaded, std::memory_order_release);
    }

    void AssetManager::ResolveDependencies(const TypedUUID &uuid,
                                            std::vector<TypedUUID> &outQueue) {
        const auto &deps = s_Registry.GetDependencies(uuid);
        for (const auto &dep : deps) {
            if (dep.depType == AssetDependencyType::Hard) {
                outQueue.push_back(dep.targetUUID);
            }
        }
    }

    size_t AssetManager::GetLoadedAssetCount() {
        std::lock_guard lock(s_CacheMutex);
        size_t count = 0;
        for (const auto &[uuid, entry] : s_Cache) {
            if (entry.controlBlock->state.load(std::memory_order_acquire) ==
                AssetLoadState::Loaded) {
                count++;
            }
        }
        return count;
    }

    size_t AssetManager::GetTotalMemoryUsage() {
        return s_TotalBytesLoaded;
        // TODO: Bessere Statistik-Tracking pro Asset
    }

    size_t AssetManager::GetCacheHitCount() {
        return s_CacheHits;
    }

} // namespace axiom
