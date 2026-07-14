#include <axiom/renderer/BindlessTextureHeap.h>
#include <axiom/renderer/rhi/BindGroup.h>
#include <axiom/renderer/rhi/IRHIBackend.h>
#include <axiom/renderer/rhi/RHITypes.h>

namespace axiom::renderer {

    rhi::RHIResult<void> BindlessTextureHeap::init(rhi::IRHIBackend &backend) {
        m_backend = &backend;

        rhi::BindGroupLayoutDesc layout;
        layout.entries.push_back(rhi::BindGroupLayoutEntry{
            .binding = 0,
            .type = rhi::BindingType::SampledTexture,
            .visibility = rhi::ShaderStage::Pixel,
            .bindless = true,
            .bindlessMaxCount = kMaxTextures,
        });

        auto layoutResult = backend.createBindGroupLayout(layout);
        if (!layoutResult)
            return std::unexpected(layoutResult.error());
        m_layout = *layoutResult;

        rhi::BindGroupDesc groupDesc;
        groupDesc.layout = m_layout;
        groupDesc.bindlessCount = kMaxTextures;
        // entries bleibt leer – Slots werden individuell über update() befüllt

        auto groupResult = backend.createBindGroup(groupDesc);
        if (!groupResult)
            return std::unexpected(groupResult.error());
        m_bindGroup = *groupResult;

        return {};
    }
} // namespace axiom::renderer