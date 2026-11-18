/**
 * @file axiom/include/axiom/renderer/rhi/ShaderDesc.h
 * @brief Declares the public interface for the shader desc.
 */

#pragma once
#include <cstdint>
#include <span>
#include "VertexLayout.h"

namespace axiom::renderer::rhi {

/**
 * @brief Defines the available shader stage values.
 * @tparam ShaderStage Template type used by this type.
 */
enum class ShaderStage : uint32_t {
    /** @brief Represents the none value. */
    none    = 0,
    /** @brief Represents the vertex value. */
    vertex  = 1u << 0,
    /** @brief Represents the pixel value. */
    pixel   = 1u << 1,
    /** @brief Represents the compute value. */
    compute = 1u << 2,
};
/**
 * @brief Defines the conversion, comparison, or access operation for this type.
 * @param a The a used by this operation.
 * @param b The b used by this operation.
 * @return The result of the operation.
 */
[[nodiscard]] constexpr ShaderStage operator|(ShaderStage a, ShaderStage b) noexcept {
    return static_cast<ShaderStage>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}
/**
 * @brief Defines the conversion, comparison, or access operation for this type.
 * @param a The a used by this operation.
 * @param b The b used by this operation.
 * @return The result of the operation.
 */
[[nodiscard]] constexpr bool operator&(ShaderStage a, ShaderStage b) noexcept {
    return (static_cast<uint32_t>(a) & static_cast<uint32_t>(b)) != 0;
}

// SPIR-V ist 32-bit-wortweise aligned, daher span<const uint32_t> statt Bytes.
/** @brief Stores the data for the shader desc. */
struct ShaderDesc {
    /** @brief Stores the stages. */
    ShaderStage stages = ShaderStage::none;
    /** @brief Stores the vertex spirv. */
    std::span<const uint32_t> vertexSpirv;
    /** @brief Stores the pixel spirv. */
    std::span<const uint32_t> pixelSpirv;
    /** @brief Stores the compute spirv. */
    std::span<const uint32_t> computeSpirv;
    /** @brief Stores the vertex layout. */
    VertexLayout vertexLayout; // nur relevant, wenn Vertex-Stage vorhanden
};

} // namespace axiom::renderer::rhi
