#pragma once
#include <vector>
#include <optional>
#include "VertexLayout.h"
#include "ShaderDesc.h"
#include "BindGroup.h"
#include "RHITypes.h"

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
    ShaderDesc shader; // enthaelt bereits VertexLayout
    PipelineState state;
    std::vector<BindGroupLayoutHandle> bindGroupLayouts;
    TextureFormat colorTargetFormat = TextureFormat::RGBA8Unorm;
    std::optional<TextureFormat> depthTargetFormat; // gesetzt wenn state.depthTest
};

} // namespace axiom::renderer::rhi
