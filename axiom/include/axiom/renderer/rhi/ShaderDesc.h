#pragma once
#include <cstdint>
#include <span>
#include "VertexLayout.h"

namespace axiom::renderer::rhi {

enum class ShaderStage : uint32_t {
    None    = 0,
    Vertex  = 1u << 0,
    Pixel   = 1u << 1,
    Compute = 1u << 2,
};
[[nodiscard]] constexpr ShaderStage operator|(ShaderStage a, ShaderStage b) noexcept {
    return static_cast<ShaderStage>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}
[[nodiscard]] constexpr bool operator&(ShaderStage a, ShaderStage b) noexcept {
    return (static_cast<uint32_t>(a) & static_cast<uint32_t>(b)) != 0;
}

// SPIR-V ist 32-bit-wortweise aligned, daher span<const uint32_t> statt Bytes.
struct ShaderDesc {
    ShaderStage stages = ShaderStage::None;
    std::span<const uint32_t> vertexSpirv;
    std::span<const uint32_t> pixelSpirv;
    std::span<const uint32_t> computeSpirv;
    VertexLayout vertexLayout; // nur relevant, wenn Vertex-Stage vorhanden
};

} // namespace axiom::renderer::rhi
