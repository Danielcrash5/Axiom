#pragma once

#include <cstdint>

namespace axiom {
    struct TextureHandle {
        uint32_t id = 0;
    };

    struct BufferHandle {
        uint32_t id = 0;
    };

    struct SamplerHandle {
        uint32_t id = 0;
    };

    struct ShaderHandle {
        uint32_t id = 0;
    };

    struct PipelineHandle {
        uint32_t id = 0;
    };
}