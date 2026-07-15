#include <axiom/renderer/MaterialSystem.h>

namespace axiom::renderer {

rhi::RHIResult<void> MaterialSystem::build(
    Material& material,
    const rhi::BindGroupLayoutDesc& bindGroupLayoutDesc,
    rhi::TextureFormat colorTargetFormat,
    std::optional<rhi::TextureFormat> depthTargetFormat) {

    const rhi::ShaderDesc* shaderDesc = m_shaders.find(material.shaderId);
    if (!shaderDesc) {
        return std::unexpected(rhi::RHIError::InvalidDescriptor);
    }

    auto layoutResult = m_backend.createBindGroupLayout(bindGroupLayoutDesc);
    if (!layoutResult) {
        return std::unexpected(layoutResult.error());
    }

    rhi::PipelineDesc pipelineDesc;
    pipelineDesc.shader = *shaderDesc;
    pipelineDesc.state = material.state;
    pipelineDesc.bindGroupLayouts = { *layoutResult };
    pipelineDesc.colorTargetFormat = colorTargetFormat;
    pipelineDesc.depthTargetFormat = depthTargetFormat;

    auto pipelineResult = m_backend.createPipeline(pipelineDesc);
    if (!pipelineResult) {
        m_backend.destroyBindGroupLayout(*layoutResult);
        return std::unexpected(pipelineResult.error());
    }

    rhi::BindGroupDesc bindGroupDesc;
    bindGroupDesc.layout = *layoutResult;
    for (auto& [name, binding] : material.params) {
        if (binding.kind != ResourceBinding::Kind::Direct) {
            // Bindless-Parameter laufen ueber ein separates Set
            // (BindlessTextureHeap::bindGroup()), nicht ueber diese BindGroup.
            continue;
        }
        rhi::BindGroupEntry entry;
        entry.binding = binding.binding;
        if (binding.buffer.valid())  entry.buffer = binding.buffer;
        if (binding.texture.valid()) entry.texture = binding.texture;
        if (binding.sampler.valid()) entry.sampler = binding.sampler;
        bindGroupDesc.entries.push_back(entry);
    }

    auto bindGroupResult = m_backend.createBindGroup(bindGroupDesc);
    if (!bindGroupResult) {
        m_backend.destroyPipeline(*pipelineResult);
        m_backend.destroyBindGroupLayout(*layoutResult);
        return std::unexpected(bindGroupResult.error());
    }

    material.m_pipeline = *pipelineResult;
    material.m_bindGroup = *bindGroupResult;
    return {};
}

} // namespace axiom::renderer
