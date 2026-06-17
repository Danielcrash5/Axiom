#pragma once

#include <cstdint>

namespace axiom {
    enum class BufferUsage {
        Vertex,
        Index,
        Uniform,
        Storage,
        Indirect,
        Transfer
    };

    struct BufferDesc {
        uint64_t size = 0;

        BufferUsage usage =
            BufferUsage::Vertex;
    };
}