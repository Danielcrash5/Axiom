/**
 * @file axiom/include/axiom/assets/AssetType.h
 * @brief Declares the public interface for the asset type.
 */

#pragma once

#include <cstdint>

namespace axiom {
    /**
     * @brief Defines the available asset type ID values.
     * @tparam AssetTypeId Template type used by this type.
     */
    enum class AssetTypeId : uint32_t {
        // Core types
        /** @brief Represents the texture value. */
        texture = 0,
        /** @brief Represents the mesh value. */
        mesh = 1,
        /** @brief Represents the material value. */
        material = 2,
        /** @brief Represents the scene value. */
        scene = 3,
        /** @brief Represents the prefab value. */
        prefab = 4,
        /** @brief Represents the script value. */
        script = 5,
        /** @brief Represents the sound effect value. */
        soundEffect = 6,
        /** @brief Represents the music value. */
        music = 7,
        /** @brief Represents the font value. */
        font = 8,

        // Legacy compatibility values
        /** @brief Represents the unknown value. */
        unknown = 0xFFFF0000,
        /** @brief Represents the shader value. */
        shader = 0xFFFF0001,
        /** @brief Represents the audio value. */
        audio = 0xFFFF0002,

        // Extensible - weitere Typen später
        /** @brief Represents the custom value. */
        custom = 0xFFFF1000,
    };

    // Compatibility alias for the legacy API surface.
    /**
     * @brief Provides the asset type type alias.
     */
    using AssetType = AssetTypeId;
} // namespace axiom