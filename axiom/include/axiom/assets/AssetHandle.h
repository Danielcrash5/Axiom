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
    // atomar über den Control-Block.
    template <typename T>
    class AssetHandle {
      public:
        AssetHandle() = default;

        AssetHandle(TypedUUID id, std::shared_ptr<AssetControlBlock> controlBlock)
            : m_ID(id), m_ControlBlock(std::move(controlBlock)) {
            if (m_ControlBlock) {
                m_ControlBlock->AddStrongRef();
            }
        }

        // Copy-Konstruktor erhöht Ref-Count
        AssetHandle(const AssetHandle &other)
            : m_ID(other.m_ID), m_ControlBlock(other.m_ControlBlock) {
            if (m_ControlBlock) {
                m_ControlBlock->AddStrongRef();
            }
        }

        // Copy-Assignment
        AssetHandle &operator=(const AssetHandle &other) {
            if (this != &other) {
                if (m_ControlBlock && m_ControlBlock->RemoveStrongRef()) {
                    // Cleanup ist Sache des AssetManager
                }
                m_ID = other.m_ID;
                m_ControlBlock = other.m_ControlBlock;
                if (m_ControlBlock) {
                    m_ControlBlock->AddStrongRef();
                }
            }
            return *this;
        }

        // Move-Konstruktor
        AssetHandle(AssetHandle &&other) noexcept
            : m_ID(other.m_ID), m_ControlBlock(std::move(other.m_ControlBlock)) {
            other.m_ID = TypedUUID();
        }

        // Move-Assignment
        AssetHandle &operator=(AssetHandle &&other) noexcept {
            if (this != &other) {
                if (m_ControlBlock && m_ControlBlock->RemoveStrongRef()) {
                    // Cleanup
                }
                m_ID = other.m_ID;
                m_ControlBlock = std::move(other.m_ControlBlock);
                other.m_ID = TypedUUID();
            }
            return *this;
        }

        // Destruktor senkt Ref-Count
        ~AssetHandle() {
            if (m_ControlBlock && m_ControlBlock->RemoveStrongRef()) {
                // Wird vom AssetManager aufgeräumt
            }
        }

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
            return m_ControlBlock->GetStrongCount();
        }

      private:
        TypedUUID m_ID;
        std::shared_ptr<AssetControlBlock> m_ControlBlock;

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
