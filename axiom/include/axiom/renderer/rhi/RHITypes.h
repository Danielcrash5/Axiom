/**
 * @file axiom/include/axiom/renderer/rhi/RHITypes.h
 * @brief Declares the public interface for the RHI types.
 */

#pragma once
#include <cstdint>
#include <string_view>
#include <expected>
#include "Handle.h"

namespace axiom::renderer::rhi {

/**
 * @brief Defines the available RHI error values.
 * @tparam RHIError Template type used by this type.
 */
enum class RHIError : uint8_t {
    /** @brief Represents the device lost value. */
    deviceLost,
    /** @brief Represents the out of memory value. */
    outOfMemory,
    /** @brief Represents the invalid descriptor value. */
    invalidDescriptor,
    /** @brief Represents the adapter request failed value. */
    adapterRequestFailed,
    /** @brief Represents the device request failed value. */
    deviceRequestFailed,
    /** @brief Represents the invalid handle value. */
    invalidHandle,
    /** @brief Represents the unknown value. */
    unknown,
};

/**
 * @brief Provides the RHI result type alias.
 * @tparam T Template type used by this alias.
 */
template <typename T>
using RHIResult = std::expected<T, RHIError>;

/**
 * @brief Defines the available texture format values.
 * @tparam TextureFormat Template type used by this type.
 */
enum class TextureFormat : uint8_t {
    /** @brief Represents the rgba8 UNORM value. */
    rgbA8Unorm,
    /** @brief Represents the bgra8 UNORM value. */
    bgrA8Unorm,
    /** @brief Represents the depth32 float value. */
    depth32Float,
    /** @brief Represents the r8 UNORM value. */
    r8Unorm,
};

/**
 * @brief Defines the available buffer usage values.
 * @tparam BufferUsage Template type used by this type.
 */
enum class BufferUsage : uint32_t {
    /** @brief Represents the none value. */
    none    = 0,
    /** @brief Represents the vertex value. */
    vertex  = 1u << 0,
    /** @brief Represents the index value. */
    index   = 1u << 1,
    /** @brief Represents the uniform value. */
    uniform = 1u << 2,
    /** @brief Represents the storage value. */
    storage = 1u << 3,
    /** @brief Represents the copy src value. */
    copySrc = 1u << 4,
    /** @brief Represents the copy dst value. */
    copyDst = 1u << 5,
};
/**
 * @brief Defines the conversion, comparison, or access operation for this type.
 * @param a The a used by this operation.
 * @param b The b used by this operation.
 * @return The result of the operation.
 */
[[nodiscard]] constexpr BufferUsage operator|(BufferUsage a, BufferUsage b) noexcept {
    return static_cast<BufferUsage>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}
/**
 * @brief Defines the conversion, comparison, or access operation for this type.
 * @param a The a used by this operation.
 * @param b The b used by this operation.
 * @return The result of the operation.
 */
[[nodiscard]] constexpr bool operator&(BufferUsage a, BufferUsage b) noexcept {
    return (static_cast<uint32_t>(a) & static_cast<uint32_t>(b)) != 0;
}

/**
 * @brief Defines the available texture usage values.
 * @tparam TextureUsage Template type used by this type.
 */
enum class TextureUsage : uint32_t {
    /** @brief Represents the none value. */
    none         = 0,
    /** @brief Represents the render target value. */
    renderTarget = 1u << 0,
    /** @brief Represents the sampled value. */
    sampled      = 1u << 1,
    /** @brief Represents the copy src value. */
    copySrc      = 1u << 2,
    /** @brief Represents the copy dst value. */
    copyDst      = 1u << 3,
};
/**
 * @brief Defines the conversion, comparison, or access operation for this type.
 * @param a The a used by this operation.
 * @param b The b used by this operation.
 * @return The result of the operation.
 */
[[nodiscard]] constexpr TextureUsage operator|(TextureUsage a, TextureUsage b) noexcept {
    return static_cast<TextureUsage>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}
/**
 * @brief Defines the conversion, comparison, or access operation for this type.
 * @param a The a used by this operation.
 * @param b The b used by this operation.
 * @return The result of the operation.
 */
[[nodiscard]] constexpr bool operator&(TextureUsage a, TextureUsage b) noexcept {
    return (static_cast<uint32_t>(a) & static_cast<uint32_t>(b)) != 0;
}

/** @brief Stores the data for the buffer desc. */
struct BufferDesc {
    /** @brief Stores the size in bytes. */
    uint64_t sizeBytes = 0;
    /** @brief Stores the usage. */
    BufferUsage usage = BufferUsage::none;
    /** @brief Stores the debug name. */
    std::string_view debugName;
};

/** @brief Stores the data for the texture desc. */
struct TextureDesc {
    /** @brief Stores the width. */
    uint32_t width = 0;
    /** @brief Stores the height. */
    uint32_t height = 0;
    /** @brief Stores the format. */
    TextureFormat format = TextureFormat::rgbA8Unorm;
    /** @brief Stores the usage. */
    TextureUsage usage = TextureUsage::none;
    /** @brief Stores the debug name. */
    std::string_view debugName;
};

/** @brief Stores the data for the texture upload desc. */
struct TextureUploadDesc {
    /** @brief Stores the width. */
    uint32_t width = 0;
    /** @brief Stores the height. */
    uint32_t height = 0;
    /** @brief Stores the bytes per row. */
    uint32_t bytesPerRow = 0; // 0 = automatisch aus width * Format-Größe berechnen
};

} // namespace axiom::renderer::rhi
