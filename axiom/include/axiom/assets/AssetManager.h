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
    class IAssetLoader {
      public:
        virtual ~IAssetLoader() = default;

        // Lädt ein Asset aus Rohbytes
        // Gibt Ownership über das gepufferte Asset ab
        virtual void *Load(const std::vector<uint8_t> &data,
                           size_t &outSizeBytes) = 0;

        // Optional: Hot-Reload-Unterstützung. Bei Erfolg gibt Reload eine neue
        // Instanz zurück und ruft Unload auf der alten Instanz selbst auf.
        virtual void *Reload(void *existing,
                             const std::vector<uint8_t> &newData,
                             size_t &outSizeBytes) {
            void *replacement = Load(newData, outSizeBytes);
            if (replacement && existing) {
                Unload(existing);
            }
            return replacement;
        }

        // Optional: Unload-Hook (für Cleanup, z.B. GPU-Ressourcen)
        virtual void Unload(void *data) {}
    };

    // Zentrale Asset-Verwaltung mit:
    // - Asynchrones Loading mit Deduplication (nur 1 Load pro UUID)
    // - Ref-Counting via Control-Block
    // - Abhängigkeitsgraph-Auflösung (hart-abhängige Assets parallel laden)
    // - Hot-Reload-Unterstützung
    class AssetManager {
      public:
        static void Init(size_t workerThreadCount = 2);
        static void Shutdown();
        [[nodiscard]] static bool IsInitialized() { return s_Initialized; }

        // === Lade-API ===

        // Asynchrone Variante: Gibt sofort einen Handle zurück, lädt im
        // Background Gibt DENSELBEN Handle zurück wenn die UUID schon geladen
        // wird/wurde
        template <typename T>
        [[nodiscard]] static AssetHandle<T> LoadAsync(TypedUUID id);

        // Synchrone Variante: Blockiert bis Ready/Failed
        // Für Editor-Tools, Startup-kritische Assets, etc.
        template <typename T>
        [[nodiscard]] static AssetHandle<T> LoadSync(TypedUUID id);

        // Synchrones Laden mit vorgegebenen Rohbytes (z.B. aus Editor)
        template <typename T>
        [[nodiscard]] static AssetHandle<T>
        LoadFromMemory(TypedUUID id, const std::vector<uint8_t> &data);

        // Entladen - senkt Ref-Count, löscht bei Ref-Count == 0
        template <typename T> static void Unload(AssetHandle<T> &handle);

        // === Abhängigkeiten ===

        // Lädt rekursiv alle hart-abhängigen Assets
        template <typename T>
        static void LoadDependencies(AssetHandle<T> handle);

        template <typename T>
        static void LoadDependenciesSync(AssetHandle<T> handle);

        // === Hot-Reload (Editor) ===
        template <typename T> static void Reload(AssetHandle<T> handle);

        // === Registry ===
        // Zugriff auf zentrale Registry (für Scans, Overrides, etc.)
        static AssetRegistry *GetRegistry() { return &s_Registry; }

        // === Statistiken ===
        static size_t GetLoadedAssetCount();
        static size_t GetTotalMemoryUsage();
        static size_t GetCacheHitCount();

        // === Loader-Registrierung ===
        static void RegisterLoader(AssetTypeId typeId, IAssetLoader *loader);

      private:
        // Control-Block mit Metadaten
        struct CacheEntry {
            std::weak_ptr<AssetControlBlock> controlBlock;
            TypedUUID uuid;
            AssetTypeId typeId;
            IAssetLoader *loader;
        };

        // Globale Caches
        static inline std::unordered_map<TypedUUID, CacheEntry> s_Cache;
        static inline std::mutex s_CacheMutex;
        static inline std::unordered_map<AssetTypeId, IAssetLoader *> s_Loaders;
        static inline std::unique_ptr<WorkerPool> s_WorkerPool;
        static inline AssetRegistry s_Registry;
        static inline size_t s_CacheHits = 0;
        static inline size_t s_TotalBytesLoaded = 0;
        static inline bool s_Initialized = false;

        // Interne Helper
        static std::shared_ptr<AssetControlBlock>
        GetOrCreateControlBlock(TypedUUID uuid);
        static IAssetLoader *GetLoader(AssetTypeId typeId);
        static void LoadAsync_Internal(TypedUUID uuid,
                                       const AssetRegistryEntry &entry);
        static void ResolveDependencies(const TypedUUID &uuid,
                                        std::vector<TypedUUID> &outQueue);
        static void LoadDependenciesInternal(
            const TypedUUID &uuid,
            std::vector<std::shared_ptr<AssetControlBlock>> &held,
            std::unordered_set<TypedUUID> &visited);
    };

    // === Template Implementierungen ===

    template <typename T>
    inline AssetHandle<T> AssetManager::LoadAsync(TypedUUID id) {
        if (!id.IsValid()) {
            return AssetHandle<T>();
        }

        auto controlBlock = GetOrCreateControlBlock(id);
        AssetLoadState expected = AssetLoadState::Unloaded;

        if (controlBlock->state.compare_exchange_strong(
                expected, AssetLoadState::Queued)) {
            // Wir sind der Erste - Job einreihen
            const AssetRegistryEntry *entry = s_Registry.Get(id);
            if (!entry) {
                controlBlock->state.store(AssetLoadState::Failed,
                                          std::memory_order_release);
                return AssetHandle<T>(id, controlBlock);
            }

            if (s_WorkerPool) {
                auto entryCopy = *entry;
                s_WorkerPool->Enqueue([uuid = id, entry = entryCopy]() {
                    LoadAsync_Internal(uuid, entry);
                });
            } else {
                LoadAsync_Internal(id, *entry);
            }
        } else {
            s_CacheHits++;
        }

        return AssetHandle<T>(id, controlBlock);
    }

    template <typename T>
    inline AssetHandle<T> AssetManager::LoadSync(TypedUUID id) {
        auto handle = LoadAsync<T>(id);

        // Spin bis Ready/Failed
        while (handle.IsLoading()) {
            std::this_thread::yield();
        }

        return handle;
    }

    template <typename T>
    inline AssetHandle<T>
    AssetManager::LoadFromMemory(TypedUUID id,
                                 const std::vector<uint8_t> &data) {
        auto controlBlock = GetOrCreateControlBlock(id);

        IAssetLoader *loader = GetLoader(id.type);
        if (!loader) {
            controlBlock->state.store(AssetLoadState::Failed,
                                      std::memory_order_release);
            return AssetHandle<T>(id, controlBlock);
        }

        size_t sizeBytes = 0;
        void *assetData = loader->Load(data, sizeBytes);
        if (!assetData) {
            controlBlock->state.store(AssetLoadState::Failed,
                                      std::memory_order_release);
            return AssetHandle<T>(id, controlBlock);
        }

        controlBlock->data = assetData;
        s_TotalBytesLoaded += sizeBytes;
        controlBlock->state.store(AssetLoadState::Loaded,
                                  std::memory_order_release);

        return AssetHandle<T>(id, controlBlock);
    }

    template <typename T>
    inline void AssetManager::Unload(AssetHandle<T> &handle) {
        handle = AssetHandle<T>();
        // Ref-Count wird durch Handle-Destruktor automatisch gesenkt
        // AssetManager räumt auf wenn Ref-Count == 0
    }

    // Async-Variante: enqueued nur, wartet nicht
    template <typename T>
    inline void AssetManager::LoadDependencies(AssetHandle<T> handle) {
        if (!handle.IsValid() || !handle.m_ControlBlock)
            return;
        handle.m_ControlBlock->heldDependencies.clear();
        for (const auto &dep : s_Registry.GetDependencies(handle.GetID())) {
            if (dep.depType != AssetDependencyType::Hard)
                continue;
            // LoadAsync<void> reiht ein und gibt sofort zurück - kein Spin-Wait
            // mehr
            auto depControlBlock = GetOrCreateControlBlock(dep.targetUUID);
            AssetLoadState expected = AssetLoadState::Unloaded;
            if (depControlBlock->state.compare_exchange_strong(
                    expected, AssetLoadState::Queued)) {
                if (const auto *entry = s_Registry.Get(dep.targetUUID)) {
                    auto entryCopy = *entry;
                    if (s_WorkerPool) {
                        s_WorkerPool->Enqueue([uuid = dep.targetUUID, entryCopy] {
                            LoadAsync_Internal(uuid, entryCopy);
                        });
                    } else {
                        LoadAsync_Internal(dep.targetUUID, entryCopy);
                    }
                }
            }
            handle.m_ControlBlock->heldDependencies.push_back(depControlBlock);
        }
    }

    // LoadDependencies synchron:
    template <typename T>
    inline void AssetManager::LoadDependenciesSync(AssetHandle<T> handle) {
        LoadDependencies(handle);
        for (auto &dep : handle.m_ControlBlock->heldDependencies) {
            while (dep->state.load(std::memory_order_acquire) ==
                       AssetLoadState::Queued ||
                   dep->state.load(std::memory_order_acquire) ==
                       AssetLoadState::Loading) {
                std::this_thread::yield();
            }
        }
    }
    template <typename T>
    inline void AssetManager::Reload(AssetHandle<T> handle) {
        if (!handle.IsValid()) {
            return;
        }

        const AssetRegistryEntry *entry = s_Registry.Get(handle.GetID());
        if (!entry) {
            return;
        }

        // Laden Sie die Datei neu
        std::vector<uint8_t> data;
        if (!VFS::ReadFile(entry->physicalPath, data)) {
            return;
        }

        IAssetLoader *loader = GetLoader(handle.GetID().type);
        if (!loader) {
            return;
        }

        auto controlBlock = handle.m_ControlBlock;
        if (!controlBlock ||
            controlBlock->state.load(std::memory_order_acquire) !=
                AssetLoadState::Loaded) {
            return;
        }

        size_t newSizeBytes = 0;
        void *newAssetData =
            loader->Reload(controlBlock->data, data, newSizeBytes);
        if (!newAssetData) {
            return;
        }

        if (s_TotalBytesLoaded >= controlBlock->sizeBytes) {
            s_TotalBytesLoaded -= controlBlock->sizeBytes;
        }
        controlBlock->data = newAssetData;
        controlBlock->sizeBytes = newSizeBytes;
        s_TotalBytesLoaded += newSizeBytes;
    }

} // namespace axiom
