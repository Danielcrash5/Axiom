/**
 * @file axiom/include/axiom/assets/AssetControlBlock.h
 * @brief Declares the public interface for the asset control block.
 */

#pragma once
#include <atomic>
#include <cstdint>
#include <memory>
#include <vector>

namespace axiom {

    /**
     * @brief Defines the available asset load state values.
     * @tparam AssetLoadState Template type used by this type.
     */
    enum class AssetLoadState : uint8_t {
        /** @brief Represents the unloaded value. */
        unloaded = 0,
        /** @brief Represents the queued value. */
        queued = 1,
        /** @brief Represents the loading value. */
        loading = 2,
        /** @brief Represents the loaded value. */
        loaded = 3,
        /** @brief Represents the failed value. */
        failed = 4,
    };

    // Zentrale Kontroll-Struktur für ein Asset.
    /** @brief Stores the data for the asset control block. */
    struct AssetControlBlock {
        // Zustand
        /** @brief Stores the state. */
        std::atomic<AssetLoadState> state{AssetLoadState::unloaded};

        // Eigentliche Asset-Daten
        /** @brief Stores the data. */
        void *data = nullptr;
        /** @brief Stores the size in bytes. */
        size_t sizeBytes = 0;

        // Harte Abhängigkeiten werden vom Parent-Asset gehalten.
        /** @brief Stores the held dependencies. */
        std::vector<std::shared_ptr<AssetControlBlock>> heldDependencies;

        /**
         * @brief Creates an instance of asset control block.
         */
        AssetControlBlock() = default;
        /**
         * @brief Releases resources owned by AssetControlBlock.
         */
        virtual ~AssetControlBlock() = default;

        // Verhindert Kopie
        /**
         * @brief Creates an instance of asset control block.
         * @param AssetControlBlock The asset control block used by this operation.
         */
        AssetControlBlock(const AssetControlBlock &) = delete;
        AssetControlBlock &operator=(const AssetControlBlock &) = delete;

    };

} // namespace axiom
