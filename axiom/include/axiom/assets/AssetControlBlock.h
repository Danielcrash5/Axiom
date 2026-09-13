#pragma once
#include <atomic>
#include <cstdint>
#include <memory>
#include <vector>

namespace axiom {

    enum class AssetLoadState : uint8_t {
        Unloaded = 0,
        Queued = 1,
        Loading = 2,
        Loaded = 3,
        Failed = 4,
    };

    // Zentrale Kontroll-Struktur für ein Asset.
    struct AssetControlBlock {
        // Zustand
        std::atomic<AssetLoadState> state{AssetLoadState::Unloaded};

        // Eigentliche Asset-Daten
        void *data = nullptr;
        size_t sizeBytes = 0;

        // Harte Abhängigkeiten werden vom Parent-Asset gehalten.
        std::vector<std::shared_ptr<AssetControlBlock>> heldDependencies;

        AssetControlBlock() = default;
        virtual ~AssetControlBlock() = default;

        // Verhindert Kopie
        AssetControlBlock(const AssetControlBlock &) = delete;
        AssetControlBlock &operator=(const AssetControlBlock &) = delete;

    };

} // namespace axiom
