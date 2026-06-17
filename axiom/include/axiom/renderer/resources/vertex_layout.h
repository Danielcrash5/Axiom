#pragma once

#include <vector>
#include <cstdint>

namespace axiom {
    enum class VertexAttributeType {
        Float,
        Vec2,
        Vec3,
        Vec4,

        Uint,
        IVecU2,
        IVecU3,
        IVecU4
    };

    struct VertexAttribute {
        const char* semantic;

        VertexAttributeType type;

        uint32_t offset;
    };

    struct VertexLayout {
        uint32_t stride = 0;

        std::vector<VertexAttribute> attributes;
    };
}