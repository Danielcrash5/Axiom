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
struct ResourceBinding {
    enum class Kind { Direct, BindlessIndex } kind = Kind::Direct;
    uint32_t binding = 0;
    rhi::BufferHandle buffer;
    rhi::TextureHandle texture;
    rhi::SamplerHandle sampler;
    uint32_t bindlessIndex = 0; // gueltig wenn kind == BindlessIndex
};

class Material {
public:
    ShaderID shaderId = ShaderID::Invalid;
    // Der string-Key ist nur ein menschenlesbarer Name fuers Material-
    // Authoring (z.B. "albedoTexture") - die tatsaechliche Zuordnung zum
    // Shader-Binding passiert ueber ResourceBinding::binding, nicht ueber
    // die Map-Reihenfolge (unordered_map hat keine garantierte Ordnung).
    std::unordered_map<std::string, ResourceBinding> params;
    rhi::PipelineState state;

    [[nodiscard]] rhi::PipelineHandle pipelineHandle() const { return m_pipeline; }
    [[nodiscard]] rhi::BindGroupHandle bindGroupHandle() const { return m_bindGroup; }
    [[nodiscard]] bool isBuilt() const { return m_pipeline.valid(); }

private:
    rhi::PipelineHandle m_pipeline;
    rhi::BindGroupHandle m_bindGroup;
    friend class MaterialSystem;
};

} // namespace axiom::renderer
