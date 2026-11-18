/**
 * @file axiom/include/axiom/renderer/Material.h
 * @brief Declares the public interface for the material.
 */

#pragma once
#include <string>
#include <unordered_map>
#include <axiom/renderer/rhi/PipelineDesc.h>
#include <axiom/renderer/rhi/BindGroup.h>
#include <axiom/renderer/rhi/Sampler.h>
#include <axiom/renderer/ShaderRegistry.h>

namespace axiom::renderer {

class MaterialSystem; // fwd

// Ein Parameter kann direktes Binding ODER Bindless-Index sein.
// "binding" muss mit dem binding-Index in der BindGroupLayoutDesc
// uebereinstimmen, die MaterialSystem::build() uebergeben wird - das
// Material selbst kennt kein Shader-Reflection, die Zuordnung ist explizit.
/** @brief Stores the data for the resource binding. */
struct ResourceBinding {
    /**
     * @brief Defines the available kind values.
     * @tparam Kind Template type used by this type.
     */
    enum class Kind { direct, bindlessIndex } kind = Kind::direct;
    /** @brief Stores the binding. */
    uint32_t binding = 0;
    /** @brief Stores the buffer. */
    rhi::BufferHandle buffer;
    /** @brief Stores the texture. */
    rhi::TextureHandle texture;
    /** @brief Stores the sampler. */
    rhi::SamplerHandle sampler;
    /** @brief Stores the bindless index. */
    uint32_t bindlessIndex = 0; // gueltig wenn kind == BindlessIndex
};

/**
 * @brief Provides the material functionality.
 * @tparam Material Template type used by this type.
 */
class Material {
public:
    /** @brief Stores the shader ID. */
    ShaderID shaderId = ShaderID::invalid;
    // Der string-Key ist nur ein menschenlesbarer Name fuers Material-
    // Authoring (z.B. "albedoTexture") - die tatsaechliche Zuordnung zum
    // Shader-Binding passiert ueber ResourceBinding::binding, nicht ueber
    // die Map-Reihenfolge (unordered_map hat keine garantierte Ordnung).
    /** @brief Stores the params. */
    std::unordered_map<std::string, ResourceBinding> params;
    /** @brief Stores the state. */
    rhi::PipelineState state;

    /**
     * @brief Returns the pipeline handle.
     * @return The result of the operation.
     */
    [[nodiscard]] rhi::PipelineHandle pipelineHandle() const { return m_pipeline; }
    /**
     * @brief Binds group handle.
     * @return The result of the operation.
     */
    [[nodiscard]] rhi::BindGroupHandle bindGroupHandle() const { return m_bindGroup; }
    /**
     * @brief Reports whether built.
     * @return The result of the operation.
     */
    [[nodiscard]] bool isBuilt() const { return m_pipeline.valid(); }

private:
    /** @brief Stores the pipeline. */
    rhi::PipelineHandle m_pipeline;
    /** @brief Stores the bind group. */
    rhi::BindGroupHandle m_bindGroup;
    friend class MaterialSystem;
};

} // namespace axiom::renderer
