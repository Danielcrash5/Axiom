#pragma once
#include <cstdint>

namespace axiom {

    struct Submesh {
        uint32_t IndexOffset;
        uint32_t IndexCount;
        uint32_t MaterialIndex; // Verweis auf Material im Model
    };

}