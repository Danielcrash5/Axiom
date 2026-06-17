#pragma once

#include "handles.h"
#include "texture.h"
#include "vertex_layout.h"

namespace axiom {
    enum class PrimitiveTopology {
        PointList,

        LineList,
        LineStrip,

        TriangleList,
        TriangleStrip
    };

    enum class CullMode {
        None,
        Front,
        Back
    };

    enum class FillMode {
        Fill,
        Wireframe
    };

    struct RasterizerState {
        CullMode cull_mode =
            CullMode::Back;

        FillMode fill_mode =
            FillMode::Fill;

        bool front_ccw = false;
    };

    struct DepthState {
        bool depth_test = true;

        bool depth_write = true;
    };

    struct BlendState {
        bool enabled = false;
    };

    struct PipelineDesc {
        ShaderHandle vertex_shader;
        ShaderHandle fragment_shader;

        ShaderHandle geometry_shader;

        ShaderHandle tess_control_shader;
        ShaderHandle tess_eval_shader;

        VertexLayout vertex_layout;

        PrimitiveTopology topology =
            PrimitiveTopology::TriangleList;

        RasterizerState rasterizer;

        DepthState depth;

        BlendState blend;

        TextureFormat color_formats[8];

        uint32_t color_attachment_count = 0;

        TextureFormat depth_format =
            TextureFormat::D24S8;
    };
}