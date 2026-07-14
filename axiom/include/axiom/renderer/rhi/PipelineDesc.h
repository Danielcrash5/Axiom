#pragma once
#include "BindGroup.h"
#include "RHITypes.h"
#include "ShaderDesc.h"
#include "VertexLayout.h"
#include <optional>
#include <vector>

namespace axiom::renderer::rhi {

enum class BlendMode { None, AlphaBlend, Additive };
enum class CullMode { None, Front, Back };
enum class PrimitiveTopology { TriangleList, LineList, LineStrip, PointList };

struct PipelineState {
    BlendMode blend = BlendMode::None;
    bool depthTest = true;
    bool depthWrite = true;
    CullMode cull = CullMode::Back;
    PrimitiveTopology topology = PrimitiveTopology::TriangleList;
};

struct PipelineDesc {
    ShaderDesc shader; // enthält bereits VertexLayout (Abschnitt 5)
    PipelineState state;
    std::vector<BindGroupLayoutHandle> bindGroupLayouts;
    TextureFormat colorTargetFormat = TextureFormat::RGBA8Unorm;
    std::optional<TextureFormat>
        depthTargetFormat; // gesetzt wenn state.depthTest
};

} // namespace axiom::renderer::rhi