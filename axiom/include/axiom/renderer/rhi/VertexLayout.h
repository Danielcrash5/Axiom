#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace axiom::renderer::rhi {

enum class VertexFormat { Float32, Float32x2, Float32x3, Float32x4, Uint32, Uint8x4Norm };

struct VertexAttribute {
    std::string semantic; // z.B. "POSITION", "TEXCOORD0", "CUSTOM_W" für 4D+
    uint32_t location = 0;
    VertexFormat format = VertexFormat::Float32x3;
    uint32_t offset = 0;
};

struct VertexLayout {
    uint32_t stride = 0;
    std::vector<VertexAttribute> attributes;
};

} // namespace axiom::renderer::rhi