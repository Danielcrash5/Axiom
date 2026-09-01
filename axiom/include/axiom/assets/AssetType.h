#pragma once

#include <cstdint>

namespace axiom {
    enum class AssetTypeId : uint32_t {
        // Core types
        Texture = 0,
        Mesh = 1,
        Material = 2,
        Scene = 3,
        Prefab = 4,
        Script = 5,
        SoundEffect = 6,
        Music = 7,
        Font = 8,

        // Legacy compatibility values
        Unknown = 0xFFFF0000,
        Shader = 0xFFFF0001,
        Audio = 0xFFFF0002,

        // Extensible - weitere Typen später
        Custom = 0xFFFF1000,
    };

    // Compatibility alias for the legacy API surface.
    using AssetType = AssetTypeId;
} // namespace axiom