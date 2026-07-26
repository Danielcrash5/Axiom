#pragma once
#include <memory>
#include <mutex>
#include <thread>
#include <unordered_map>

#include "AssetHandle.h"
#include "AssetLoaderRegistry.h"
#include "AssetRegistry.h"
#include <axiom/threading/WorkerPool.h>

namespace axiom {

    // Asynchron, cache-deduplizierend. Ersetzt die alte synchrone
    // AssetManager::Get<T>() - Streaming braucht einen Zwischenzustand
    // ("wird gerade geladen"), den ein direkter shared_ptr-Rueckgabewert
    // nicht abbilden konnte.
    class AssetManager {
      public:
        static void Init(size_t workerThreadCount = 2);
        static void Shutdown();

        // Gibt sofort einen Handle zurueck. Ist die AssetID neu, wird ein Job
        // im Hintergrund gestartet (WorkerPool). Ist sie schon im Cache (auch
        // waehrend sie noch laedt), wird derselbe Slot zurueckgegeben - kein
        // doppeltes Laden bei mehreren Anfragen fuer dieselbe ID.
        template <typename T> [[nodiscard]] static AssetHandle<T> RequestLoad(AssetID id);

        // Blockiert bis Ready/Failed. Fuer Faelle, wo synchrones Verhalten
        // bewusst gewuenscht ist (z.B. Editor-Tools, Startup-kritische Assets).
        template <typename T> [[nodiscard]] static std::shared_ptr<T> GetBlocking(AssetID id);

      private:
        [[nodiscard]] static std::shared_ptr<AssetSlot> GetOrCreateSlot(AssetID id);

        static inline std::unordered_map<AssetID, std::shared_ptr<AssetSlot>> s_Slots;
        static inline std::mutex s_SlotsMutex; // schuetzt NUR s_Slots selbst, nicht Slot-Inhalte
        static inline std::unique_ptr<WorkerPool> s_WorkerPool;
    };

    // --- Templates ---

    template <typename T> AssetHandle<T> AssetManager::RequestLoad(AssetID id) {
        auto slot = GetOrCreateSlot(id);

        AssetLoadState expected = AssetLoadState::Unloaded;
        if (slot->state.compare_exchange_strong(expected, AssetLoadState::Loading)) {
            // Wir sind der Erste, der diese ID anfragt - Job einreihen.
            // Metadata wird HIER (Aufrufer-Thread) synchron geholt und per
            // Wert in den Job kopiert, damit der Worker-Thread nicht auf
            // AssetRegistrys statische Maps zugreift (die sind nicht
            // synchronisiert - bewusst unangetastet gelassen, siehe unten).
            const AssetMetadata *metadata = AssetRegistry::Get(id);
            if (!metadata) {
                slot->state.store(AssetLoadState::Failed, std::memory_order_release);
                return AssetHandle<T>(id, slot);
            }
            AssetMetadata metadataCopy = *metadata;

            if (s_WorkerPool) {
                s_WorkerPool->Enqueue([id, metadataCopy, slot] {
                    const AssetLoaderFn *loader = AssetLoaderRegistry::Find(metadataCopy.Type);
                    if (!loader) {
                        slot->state.store(AssetLoadState::Failed, std::memory_order_release);
                        return;
                    }
                    std::shared_ptr<Asset> loaded = (*loader)(metadataCopy);
                    if (!loaded) {
                        slot->state.store(AssetLoadState::Failed, std::memory_order_release);
                        return;
                    }
                    slot->asset = std::move(loaded); // Schreiben VOR dem Release-Store unten
                    slot->state.store(AssetLoadState::Ready, std::memory_order_release);
                });
            }
        }
        // War state schon != Unloaded (Loading/Ready/Failed), machen wir
        // nichts - derselbe Slot wird einfach zurueckgegeben (Deduplizierung).

        return AssetHandle<T>(id, slot);
    }

    template <typename T> std::shared_ptr<T> AssetManager::GetBlocking(AssetID id) {
        auto handle = RequestLoad<T>(id);
        while (!handle.IsReady()) {
            if (handle.IsFailed()) return nullptr;
            std::this_thread::yield();
        }
        return handle.Get();
    }

} // namespace axiom
