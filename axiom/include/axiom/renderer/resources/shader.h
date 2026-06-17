#pragma once

#include <string>

namespace axiom {
    enum class ShaderStage {
        Vertex,
        Fragment,

        Geometry,

        TessControl,
        TessEvaluation,

        Compute
    };

    struct ShaderDesc {
        std::string name;

        ShaderStage stage;
    };
}