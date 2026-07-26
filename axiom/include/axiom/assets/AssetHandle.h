#pragma once
#include <atomic>
#include <memory>

#include "Asset.h"
#include "AssetID.h"
#include "AssetLoadState.h"

namespace axiom {

    // Von AssetManager verwaltet, von AssetHandle<T> nur gelesen. `state`
    // wird per Release-Store gesetzt NACHDEM `asset` geschrieben wurde
    // (Publish-Idiom) - AssetHandle::get() liest `state` per Acquire-Load und
    // greift nur bei Ready auf `asset` zu. Threadsicher ohne Mutex pro Zugriff.
    struct AssetSlot {
        std::atomic<AssetLoadState> state{AssetLoadState::Unloaded};
        std::shared_ptr<Asset> asset; // gueltig ERST wenn state == Ready gelesen wurde
    };

    // Leichtgewichtiger, kopierbarer Griff auf einen Asset-Ladevorgang.
    // Mehrere AssetHandle<T>-Kopien koennen denselben Slot beobachten - der
    // Ladevorgang selbst passiert nur einmal pro AssetID (AssetManager
    // dedupliziert ueber getOrCreateSlot()).
    template <typename T>
    class AssetHandle {
      public:
        AssetHandle() = default;
        AssetHandle(AssetID id, std::shared_ptr<AssetSlot> slot)
            : m_ID(id), m_Slot(std::move(slot)) {}

        [[nodiscard]] AssetID GetID() const { return m_ID; }

        [[nodiscard]] AssetLoadState GetState() const {
            if (!m_Slot) return AssetLoadState::Unloaded;
            return m_Slot->state.load(std::memory_order_acquire);
        }

        [[nodiscard]] bool IsReady() const {
            return GetState() == AssetLoadState::Ready;
        }

        [[nodiscard]] bool IsFailed() const {
            return GetState() == AssetLoadState::Failed;
        }

        // nullptr, solange GetState() != Ready
        [[nodiscard]] std::shared_ptr<T> Get() const {
            if (GetState() != AssetLoadState::Ready) return nullptr;
            return std::static_pointer_cast<T>(m_Slot->asset);
        }

      private:
        AssetID m_ID{};
        std::shared_ptr<AssetSlot> m_Slot;
    };

} // namespace axiom
