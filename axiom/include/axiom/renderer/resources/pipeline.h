#pragma once

#include "handles.h"

namespace axiom {
    struct PipelineDesc {
        ShaderHandle vertex_shader;
        ShaderHandle fragment_shader;
    };
}