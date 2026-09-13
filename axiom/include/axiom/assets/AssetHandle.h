#pragma once
#include <functional>
#include <memory>
#include <vector>

#include "AssetControlBlock.h"
#include "TypedUUID.h"

namespace axiom {

    // Leichtgewichtiger, kopierbarer Griff auf einen Asset-Ladevorgang.
    // Mehrere AssetHandle<T>-Kopien können denselben Asset beobachten.
    // Der Ladevorgang selbst passiert nur einmal pro TypedUUID (AssetManager
    // dedupliziert über Caching).
    //
    // Thread-Safety: Handle selbst ist kopiersicher, Ref-Counting erfolgt
    // durch shared_ptr.
    template <typename T>
    class AssetHandle {
      public:
        AssetHandle() = default;

        AssetHandle(TypedUUID id, std::shared_ptr<AssetControlBlock> controlBlock)
            : m_ID(id), m_ControlBlock(std::move(controlBlock)) {}

        AssetHandle(const AssetHandle &) = default;
        AssetHandle &operator=(const AssetHandle &) = default;
        AssetHandle(AssetHandle &&) noexcept = default;
        AssetHandle &operator=(AssetHandle &&) noexcept = default;
        ~AssetHandle() = default;

        [[nodiscard]] TypedUUID GetID() const { return m_ID; }

        [[nodiscard]] AssetLoadState GetState() const {
            if (!m_ControlBlock)
                return AssetLoadState::Unloaded;
            return m_ControlBlock->state.load(std::memory_order_acquire);
        }

        [[nodiscard]] bool IsReady() const {
            return GetState() == AssetLoadState::Loaded;
        }

        [[nodiscard]] bool IsFailed() const {
            return GetState() == AssetLoadState::Failed;
        }

        [[nodiscard]] bool IsLoading() const {
            return GetState() == AssetLoadState::Loading ||
                   GetState() == AssetLoadState::Queued;
        }

        // Gibt Zugriff auf das Asset - nullptr solange GetState() != Loaded
        [[nodiscard]] T *Get() const {
            if (GetState() != AssetLoadState::Loaded)
                return nullptr;
            return static_cast<T *>(m_ControlBlock->data);
        }

        // Callback when asset is loaded (or immediately if already loaded)
        void OnLoaded(std::function<void(AssetHandle<T>)> callback) const {
            if (GetState() == AssetLoadState::Loaded) {
                callback(*this);
            } else if (GetState() != AssetLoadState::Failed) {
                // TODO: Callback-Registry für noch-nicht-geladene Assets
            }
        }

        [[nodiscard]] bool IsValid() const { return m_ControlBlock != nullptr; }

        [[nodiscard]] uint32_t GetRefCount() const {
            if (!m_ControlBlock)
                return 0;
            return static_cast<uint32_t>(m_ControlBlock.use_count());
        }

      private:
        TypedUUID m_ID;
        std::shared_ptr<AssetControlBlock> m_ControlBlock;

                friend class AssetManager;
        template <typename>
        friend class WeakAssetHandle;
    };

    // WeakAssetHandle - für später (Editor-Preview, keine Ownership)
    template <typename T>
    class WeakAssetHandle {
      public:
        WeakAssetHandle() = default;

        WeakAssetHandle(const AssetHandle<T> &handle)
            : m_ID(handle.GetID()), m_ControlBlock(handle.m_ControlBlock) {}

        [[nodiscard]] AssetHandle<T> Lock() const {
            if (auto cb = m_ControlBlock.lock()) {
                return AssetHandle<T>(m_ID, cb);
            }
            return AssetHandle<T>();
        }

      private:
        TypedUUID m_ID;
        std::weak_ptr<AssetControlBlock> m_ControlBlock;
    };

} // namespace axiom
