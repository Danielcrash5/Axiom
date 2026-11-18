/**
 * @file axiom/include/axiom/assets/AssetHandle.h
 * @brief Declares the public interface for the asset handle.
 */

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
    /**
     * @brief Provides the asset handle functionality.
     * @tparam T Template type used by this type.
     * @tparam AssetHandle Template type used by this type.
     */
    template <typename T>
    class AssetHandle {
      public:
        /**
         * @brief Creates an instance of asset handle.
         */
        AssetHandle() = default;

        /**
         * @brief Creates an instance of asset handle.
         * @param id The ID used by this operation.
         * @param controlBlock The control block used by this operation.
         */
        AssetHandle(TypedUUID id, std::shared_ptr<AssetControlBlock> controlBlock)
            : m_id(id), m_controlBlock(std::move(controlBlock)) {}

        /**
         * @brief Creates an instance of asset handle.
         * @param AssetHandle The asset handle used by this operation.
         */
        AssetHandle(const AssetHandle &) = default;
        AssetHandle &operator=(const AssetHandle &) = default;
        /**
         * @brief Creates an instance of asset handle.
         */
        AssetHandle(AssetHandle &&) noexcept = default;
        AssetHandle &operator=(AssetHandle &&) noexcept = default;
        /**
         * @brief Releases resources owned by AssetHandle.
         */
        ~AssetHandle() = default;

        /**
         * @brief Returns the ID.
         * @return The result of the operation.
         */
        [[nodiscard]] TypedUUID getId() const { return m_id; }

        /**
         * @brief Returns the state.
         * @return The result of the operation.
         */
        [[nodiscard]] AssetLoadState getState() const {
            if (!m_controlBlock)
                return AssetLoadState::unloaded;
            return m_controlBlock->state.load(std::memory_order_acquire);
        }

        /**
         * @brief Reports whether ready.
         * @return The result of the operation.
         */
        [[nodiscard]] bool isReady() const {
            return getState() == AssetLoadState::loaded;
        }

        /**
         * @brief Reports whether failed.
         * @return The result of the operation.
         */
        [[nodiscard]] bool isFailed() const {
            return getState() == AssetLoadState::failed;
        }

        /**
         * @brief Reports whether loading.
         * @return The result of the operation.
         */
        [[nodiscard]] bool isLoading() const {
            return getState() == AssetLoadState::loading ||
                   getState() == AssetLoadState::queued;
        }

        // Gibt Zugriff auf das Asset - nullptr solange GetState() != Loaded
        /**
         * @brief Returns the requested value.
         * @return The result of the operation.
         */
        [[nodiscard]] T *get() const {
            if (getState() != AssetLoadState::loaded)
                return nullptr;
            return static_cast<T *>(m_controlBlock->data);
        }

        // Callback when asset is loaded (or immediately if already loaded)
        /**
         * @brief Handles the loaded callback.
         * @param callback The callback used by this operation.
         */
        void onLoaded(std::function<void(AssetHandle<T>)> callback) const {
            if (getState() == AssetLoadState::loaded) {
                callback(*this);
            } else if (getState() != AssetLoadState::failed) {
                // TODO: Callback-Registry für noch-nicht-geladene Assets
            }
        }

        /**
         * @brief Reports whether valid.
         * @return The result of the operation.
         */
        [[nodiscard]] bool isValid() const { return m_controlBlock != nullptr; }

        /**
         * @brief Returns the ref count.
         * @return The result of the operation.
         */
        [[nodiscard]] uint32_t getRefCount() const {
            if (!m_controlBlock)
                return 0;
            return static_cast<uint32_t>(m_controlBlock.use_count());
        }

      private:
        /** @brief Stores the ID. */
        TypedUUID m_id;
        /** @brief Stores the control block. */
        std::shared_ptr<AssetControlBlock> m_controlBlock;

                friend class AssetManager;
        template <typename>
        friend class WeakAssetHandle;
    };

    // WeakAssetHandle - für später (Editor-Preview, keine Ownership)
    /**
     * @brief Provides the weak asset handle functionality.
     * @tparam T Template type used by this type.
     * @tparam WeakAssetHandle Template type used by this type.
     */
    template <typename T>
    class WeakAssetHandle {
      public:
        /**
         * @brief Creates an instance of weak asset handle.
         */
        WeakAssetHandle() = default;

        /**
         * @brief Creates an instance of weak asset handle.
         * @param handle The handle used by this operation.
         */
        WeakAssetHandle(const AssetHandle<T> &handle)
            : m_id(handle.GetID()), m_controlBlock(handle.m_ControlBlock) {}

        /**
         * @brief Acquires the lock that protects shared state.
         * @return The result of the operation.
         */
        [[nodiscard]] AssetHandle<T> lock() const {
            if (auto cb = m_controlBlock.lock()) {
                return AssetHandle<T>(m_id, cb);
            }
            return AssetHandle<T>();
        }

      private:
        /** @brief Stores the ID. */
        TypedUUID m_id;
        /** @brief Stores the control block. */
        std::weak_ptr<AssetControlBlock> m_controlBlock;
    };

} // namespace axiom
