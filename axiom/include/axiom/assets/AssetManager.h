/**
 * @file axiom/include/axiom/assets/AssetManager.h
 * @brief Declares the public interface for the asset manager.
 */

#pragma once
#include <axiom/assets/AssetControlBlock.h>
#include <axiom/assets/AssetHandle.h>
#include <axiom/assets/AssetRegistry.h>
#include <axiom/assets/TypedUUID.h>
#include <axiom/assets/VFS.h>
#include <axiom/threading/WorkerPool.h>
#include <functional>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <unordered_set>

namespace axiom {

    // Basis-Interface für Asset-Loader
    /**
     * @brief Defines the interface for the asset loader.
     * @tparam IAssetLoader Template type used by this type.
     */
    class IAssetLoader {
      public:
        /**
         * @brief Releases resources owned by IAssetLoader.
         */
        virtual ~IAssetLoader() = default;

        // Lädt ein Asset aus Rohbytes
        // Gibt Ownership über das gepufferte Asset ab
        /**
         * @brief Loads the requested resource.
         * @param data The data used by this operation.
         * @param[out] outSizeBytes Receives the output size in bytes.
         * @return The result of the operation.
         */
        virtual void *load(const std::vector<uint8_t> &data,
                           size_t &outSizeBytes) = 0;

        // Optional: Hot-Reload-Unterstützung. Bei Erfolg gibt Reload eine neue
        // Instanz zurück und ruft Unload auf der alten Instanz selbst auf.
        /**
         * @brief Reloads the requested resource.
         * @param existing The existing used by this operation.
         * @param newData The new data used by this operation.
         * @param[out] outSizeBytes Receives the output size in bytes.
         * @return The result of the operation.
         */
        virtual void *reload(void *existing,
                             const std::vector<uint8_t> &newData,
                             size_t &outSizeBytes) {
            void *replacement = load(newData, outSizeBytes);
            if (replacement && existing) {
                unload(existing);
            }
            return replacement;
        }

        // Optional: Unload-Hook (für Cleanup, z.B. GPU-Ressourcen)
        /**
         * @brief Releases the loaded resource.
         * @param data The data used by this operation.
         */
        virtual void unload(void *data) {}
    };

    // Zentrale Asset-Verwaltung mit:
    // - Asynchrones Loading mit Deduplication (nur 1 Load pro UUID)
    // - Ref-Counting via Control-Block
    // - Abhängigkeitsgraph-Auflösung (hart-abhängige Assets parallel laden)
    // - Hot-Reload-Unterstützung
    /**
     * @brief Loads, caches, reloads, and releases typed assets.
     * @tparam AssetManager Template type used by this type.
     */
    class AssetManager {
      public:
        /**
         * @brief Initializes the subsystem.
         * @param workerThreadCount The worker thread count used by this operation.
         */
        static void init(size_t workerThreadCount = 2);
        /**
         * @brief Shuts down the subsystem.
         */
        static void shutdown();
        /**
         * @brief Reports whether initialized.
         * @return The result of the operation.
         */
        [[nodiscard]] static bool isInitialized() { return sInitialized; }

        // === Lade-API ===

        // Asynchrone Variante: Gibt sofort einen Handle zurück, lädt im
        // Background Gibt DENSELBEN Handle zurück wenn die UUID schon geladen
        // wird/wurde
        /**
         * @brief Loads async.
         * @tparam T Template type used by this operation.
         * @param id The ID used by this operation.
         * @return The result of the operation.
         */
        template <typename T>
        [[nodiscard]] static AssetHandle<T> loadAsync(TypedUUID id);

        // Synchrone Variante: Blockiert bis Ready/Failed
        // Für Editor-Tools, Startup-kritische Assets, etc.
        /**
         * @brief Loads sync.
         * @tparam T Template type used by this operation.
         * @param id The ID used by this operation.
         * @return The result of the operation.
         */
        template <typename T>
        [[nodiscard]] static AssetHandle<T> loadSync(TypedUUID id);

        // Synchrones Laden mit vorgegebenen Rohbytes (z.B. aus Editor)
        /**
         * @brief Loads from memory.
         * @tparam T Template type used by this operation.
         * @param id The ID used by this operation.
         * @param data The data used by this operation.
         * @return The result of the operation.
         */
        template <typename T>
        [[nodiscard]] static AssetHandle<T>
        loadFromMemory(TypedUUID id, const std::vector<uint8_t> &data);

        // Entladen - senkt Ref-Count, löscht bei Ref-Count == 0
        /**
         * @brief Releases the loaded resource.
         * @tparam T Template type used by this operation.
         * @param handle The handle used by this operation.
         */
        template <typename T> static void unload(AssetHandle<T> &handle);

        // === Abhängigkeiten ===

        // Lädt rekursiv alle hart-abhängigen Assets
        /**
         * @brief Loads dependencies.
         * @tparam T Template type used by this operation.
         * @param handle The handle used by this operation.
         */
        template <typename T>
        static void loadDependencies(AssetHandle<T> handle);

        /**
         * @brief Loads dependencies sync.
         * @tparam T Template type used by this operation.
         * @param handle The handle used by this operation.
         */
        template <typename T>
        static void loadDependenciesSync(AssetHandle<T> handle);

        // === Hot-Reload (Editor) ===
        /**
         * @brief Reloads the requested resource.
         * @tparam T Template type used by this operation.
         * @param handle The handle used by this operation.
         */
        template <typename T> static void reload(AssetHandle<T> handle);

        // === Registry ===
        // Zugriff auf zentrale Registry (für Scans, Overrides, etc.)
        /**
         * @brief Returns the registry.
         * @return The result of the operation.
         */
        static AssetRegistry *getRegistry() { return &sRegistry; }

        // === Statistiken ===
        /**
         * @brief Returns the loaded asset count.
         * @return The result of the operation.
         */
        static size_t getLoadedAssetCount();
        /**
         * @brief Returns the total memory usage.
         * @return The result of the operation.
         */
        static size_t getTotalMemoryUsage();
        /**
         * @brief Returns the cache hit count.
         * @return The result of the operation.
         */
        static size_t getCacheHitCount();

        // === Loader-Registrierung ===
        /**
         * @brief Registers loader.
         * @param typeId The type ID used by this operation.
         * @param loader The loader used by this operation.
         */
        static void registerLoader(AssetTypeId typeId, IAssetLoader *loader);

      private:
        // Control-Block mit Metadaten
        /** @brief Stores the data for the cache entry. */
        struct CacheEntry {
            /** @brief Stores the control block. */
            std::weak_ptr<AssetControlBlock> controlBlock;
            /** @brief Stores the UUID. */
            TypedUUID uuid;
            /** @brief Stores the type ID. */
            AssetTypeId typeId;
            /** @brief Stores the loader. */
            IAssetLoader *loader;
        };

        // Globale Caches
        /** @brief Stores the cache. */
        static inline std::unordered_map<TypedUUID, CacheEntry> sCache;
        /** @brief Stores the cache mutex. */
        static inline std::mutex sCacheMutex;
        /** @brief Stores the loaders. */
        static inline std::unordered_map<AssetTypeId, IAssetLoader *> sLoaders;
        /** @brief Stores the worker pool. */
        static inline std::unique_ptr<WorkerPool> sWorkerPool;
        /** @brief Stores the registry. */
        static inline AssetRegistry sRegistry;
        /** @brief Stores the cache hits. */
        static inline size_t sCacheHits = 0;
        /** @brief Stores the total bytes loaded. */
        static inline size_t sTotalBytesLoaded = 0;
        /** @brief Stores the initialized. */
        static inline bool sInitialized = false;

        // Interne Helper
        /**
         * @brief Returns the or create control block.
         * @param uuid The UUID used by this operation.
         * @return The result of the operation.
         */
        static std::shared_ptr<AssetControlBlock>
        getOrCreateControlBlock(TypedUUID uuid);
        /**
         * @brief Returns the loader.
         * @param typeId The type ID used by this operation.
         * @return The result of the operation.
         */
        static IAssetLoader *getLoader(AssetTypeId typeId);
        /**
         * @brief Loads async internal.
         * @param uuid The UUID used by this operation.
         * @param entry The entry used by this operation.
         */
        static void loadAsyncInternal(TypedUUID uuid,
                                       const AssetRegistryEntry &entry);
        /**
         * @brief Resolves dependencies.
         * @param uuid The UUID used by this operation.
         * @param[out] outQueue Receives the queue value.
         */
        static void resolveDependencies(const TypedUUID &uuid,
                                        std::vector<TypedUUID> &outQueue);
        /**
         * @brief Loads dependencies internal.
         * @param uuid The UUID used by this operation.
         * @param held The held used by this operation.
         * @param visited The visited used by this operation.
         */
        static void loadDependenciesInternal(
            const TypedUUID &uuid,
            std::vector<std::shared_ptr<AssetControlBlock>> &held,
            std::unordered_set<TypedUUID> &visited);
    };

    // === Template Implementierungen ===

    /**
     * @brief Loads async.
     * @tparam T Template type used by this operation.
     * @param id The ID used by this operation.
     * @return The result of the operation.
     */
    template <typename T>
    inline AssetHandle<T> AssetManager::loadAsync(TypedUUID id) {
        if (!id.isValid()) {
            return AssetHandle<T>();
        }

        auto controlBlock = getOrCreateControlBlock(id);
        AssetLoadState expected = AssetLoadState::unloaded;

        if (controlBlock->state.compare_exchange_strong(
                expected, AssetLoadState::queued)) {
            // Wir sind der Erste - Job einreihen
            const AssetRegistryEntry *entry = sRegistry.get(id);
            if (!entry) {
                controlBlock->state.store(AssetLoadState::failed,
                                          std::memory_order_release);
                return AssetHandle<T>(id, controlBlock);
            }

            if (sWorkerPool) {
                auto entryCopy = *entry;
                sWorkerPool->enqueue([uuid = id, entry = entryCopy]() {
                    loadAsyncInternal(uuid, entry);
                });
            } else {
                loadAsyncInternal(id, *entry);
            }
        } else {
            sCacheHits++;
        }

        return AssetHandle<T>(id, controlBlock);
    }

    /**
     * @brief Loads sync.
     * @tparam T Template type used by this operation.
     * @param id The ID used by this operation.
     * @return The result of the operation.
     */
    template <typename T>
    inline AssetHandle<T> AssetManager::loadSync(TypedUUID id) {
        auto handle = LoadAsync<T>(id);

        // Spin bis Ready/Failed
        while (handle.isLoading()) {
            std::this_thread::yield();
        }

        return handle;
    }

    /**
     * @brief Loads from memory.
     * @tparam T Template type used by this operation.
     * @param id The ID used by this operation.
     * @param data The data used by this operation.
     * @return The result of the operation.
     */
    template <typename T>
    inline AssetHandle<T>
    AssetManager::loadFromMemory(TypedUUID id,
                                 const std::vector<uint8_t> &data) {
        auto controlBlock = getOrCreateControlBlock(id);

        IAssetLoader *loader = getLoader(id.type);
        if (!loader) {
            controlBlock->state.store(AssetLoadState::failed,
                                      std::memory_order_release);
            return AssetHandle<T>(id, controlBlock);
        }

        size_t sizeBytes = 0;
        void *assetData = loader->load(data, sizeBytes);
        if (!assetData) {
            controlBlock->state.store(AssetLoadState::failed,
                                      std::memory_order_release);
            return AssetHandle<T>(id, controlBlock);
        }

        controlBlock->data = assetData;
        sTotalBytesLoaded += sizeBytes;
        controlBlock->state.store(AssetLoadState::loaded,
                                  std::memory_order_release);

        return AssetHandle<T>(id, controlBlock);
    }

    /**
     * @brief Releases the loaded resource.
     * @tparam T Template type used by this operation.
     * @param handle The handle used by this operation.
     */
    template <typename T>
    inline void AssetManager::unload(AssetHandle<T> &handle) {
        handle = AssetHandle<T>();
        // Ref-Count wird durch Handle-Destruktor automatisch gesenkt
        // AssetManager räumt auf wenn Ref-Count == 0
    }

    // Async-Variante: enqueued nur, wartet nicht
    /**
     * @brief Loads dependencies.
     * @tparam T Template type used by this operation.
     * @param handle The handle used by this operation.
     */
    template <typename T>
    inline void AssetManager::loadDependencies(AssetHandle<T> handle) {
        if (!handle.isValid() || !handle.m_controlBlock)
            return;
        handle.m_controlBlock->heldDependencies.clear();
        for (const auto &dep : sRegistry.getDependencies(handle.getId())) {
            if (dep.depType != AssetDependencyType::hard)
                continue;
            // LoadAsync<void> reiht ein und gibt sofort zurück - kein Spin-Wait
            // mehr
            auto depControlBlock = getOrCreateControlBlock(dep.targetUUID);
            AssetLoadState expected = AssetLoadState::unloaded;
            if (depControlBlock->state.compare_exchange_strong(
                    expected, AssetLoadState::queued)) {
                if (const auto *entry = sRegistry.get(dep.targetUUID)) {
                    auto entryCopy = *entry;
                    if (sWorkerPool) {
                        sWorkerPool->enqueue([uuid = dep.targetUUID, entryCopy] {
                            loadAsyncInternal(uuid, entryCopy);
                        });
                    } else {
                        loadAsyncInternal(dep.targetUUID, entryCopy);
                    }
                }
            }
            handle.m_controlBlock->heldDependencies.push_back(depControlBlock);
        }
    }

    // LoadDependencies synchron:
    /**
     * @brief Loads dependencies sync.
     * @tparam T Template type used by this operation.
     * @param handle The handle used by this operation.
     */
    template <typename T>
    inline void AssetManager::loadDependenciesSync(AssetHandle<T> handle) {
        LoadDependencies(handle);
        for (auto &dep : handle.m_ControlBlock->heldDependencies) {
            while (dep->state.load(std::memory_order_acquire) ==
                       AssetLoadState::queued ||
                   dep->state.load(std::memory_order_acquire) ==
                       AssetLoadState::loading) {
                std::this_thread::yield();
            }
        }
    }
    /**
     * @brief Reloads the requested resource.
     * @tparam T Template type used by this operation.
     * @param handle The handle used by this operation.
     */
    template <typename T>
    inline void AssetManager::reload(AssetHandle<T> handle) {
        if (!handle.isValid()) {
            return;
        }

        const AssetRegistryEntry *entry = sRegistry.get(handle.getId());
        if (!entry) {
            return;
        }

        // Laden Sie die Datei neu
        std::vector<uint8_t> data;
        if (!VFS::readFile(entry->physicalPath, data)) {
            return;
        }

        IAssetLoader *loader = getLoader(handle.getId().type);
        if (!loader) {
            return;
        }

        auto controlBlock = handle.m_controlBlock;
        if (!controlBlock ||
            controlBlock->state.load(std::memory_order_acquire) !=
                AssetLoadState::loaded) {
            return;
        }

        size_t newSizeBytes = 0;
        void *newAssetData =
            loader->reload(controlBlock->data, data, newSizeBytes);
        if (!newAssetData) {
            return;
        }

        if (sTotalBytesLoaded >= controlBlock->sizeBytes) {
            sTotalBytesLoaded -= controlBlock->sizeBytes;
        }
        controlBlock->data = newAssetData;
        controlBlock->sizeBytes = newSizeBytes;
        sTotalBytesLoaded += newSizeBytes;
    }

} // namespace axiom
