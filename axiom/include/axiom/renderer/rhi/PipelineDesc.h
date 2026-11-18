/**
 * @file axiom/include/axiom/renderer/rhi/PipelineDesc.h
 * @brief Declares the public interface for the pipeline desc.
 */

#pragma once
#include <vector>
#include <optional>
#include "VertexLayout.h"
#include "ShaderDesc.h"
#include "BindGroup.h"
#include "RHITypes.h"

namespace axiom::renderer::rhi {

/**
 * @brief Defines the available blend mode values.
 * @tparam BlendMode Template type used by this type.
 */
enum class BlendMode { none, alphaBlend, additive };
/**
 * @brief Defines the available cull mode values.
 * @tparam CullMode Template type used by this type.
 */
enum class CullMode { none, front, back };
/**
 * @brief Defines the available primitive topology values.
 * @tparam PrimitiveTopology Template type used by this type.
 */
enum class PrimitiveTopology { triangleList, lineList, lineStrip, pointList };

/** @brief Stores the data for the pipeline state. */
struct PipelineState {
    /** @brief Stores the blend. */
    BlendMode blend = BlendMode::none;
    /** @brief Stores the depth test. */
    bool depthTest = true;
    /** @brief Stores the depth write. */
    bool depthWrite = true;
    /** @brief Stores the cull. */
    CullMode cull = CullMode::back;
    /** @brief Stores the topology. */
    PrimitiveTopology topology = PrimitiveTopology::triangleList;
};

/** @brief Stores the data for the pipeline desc. */
struct PipelineDesc {
    /** @brief Stores the shader. */
    ShaderDesc shader; // enthaelt bereits VertexLayout
    /** @brief Stores the state. */
    PipelineState state;
    /** @brief Stores the bind group layouts. */
    std::vector<BindGroupLayoutHandle> bindGroupLayouts;
    /** @brief Stores the color target format. */
    TextureFormat colorTargetFormat = TextureFormat::rgbA8Unorm;
    /** @brief Stores the depth target format. */
    std::optional<TextureFormat> depthTargetFormat; // gesetzt wenn state.depthTest
};

} // namespace axiom::renderer::rhi
