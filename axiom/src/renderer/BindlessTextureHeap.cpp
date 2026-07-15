#include <axiom/renderer/BindlessTextureHeap.h>

namespace axiom::renderer {

rhi::RHIResult<void> BindlessTextureHeap::init(rhi::IRHIBackend& backend) {
    m_backend = &backend;

    rhi::BindGroupLayoutDesc layoutDesc;
    rhi::BindGroupLayoutEntry entry;
    entry.binding = 0;
    entry.type = rhi::BindingType::SampledTexture;
    entry.visibility = rhi::ShaderStage::Pixel;
    entry.bindless = true;
    entry.bindlessMaxCount = kMaxTextures;
    layoutDesc.entries.push_back(entry);

    auto layoutResult = backend.createBindGroupLayout(layoutDesc);
    if (!layoutResult) {
        return std::unexpected(layoutResult.error());
    }
    m_layout = *layoutResult;

    rhi::BindGroupDesc groupDesc;
    groupDesc.layout = m_layout;
    groupDesc.bindlessCount = kMaxTextures;
    // entries bleibt leer - Slots werden individuell ueber update() befuellt.

    auto groupResult = backend.createBindGroup(groupDesc);
    if (!groupResult) {
        backend.destroyBindGroupLayout(m_layout);
        return std::unexpected(groupResult.error());
    }
    m_bindGroup = *groupResult;

    return {};
}

rhi::RHIResult<uint32_t> BindlessTextureHeap::allocate(rhi::TextureHandle texture) {
    uint32_t slot;
    if (!m_freeSlots.empty()) {
        slot = m_freeSlots.back();
        m_freeSlots.pop_back();
    } else {
        if (m_nextSlot >= kMaxTextures) {
            return std::unexpected(rhi::RHIError::OutOfMemory);
        }
        slot = m_nextSlot++;
    }

    if (auto result = update(slot, texture); !result) {
        return std::unexpected(result.error());
    }
    return slot;
}

void BindlessTextureHeap::free(uint32_t index) {
    m_freeSlots.push_back(index);
    // Bewusst KEIN Descriptor-Reset auf "leer" - PARTIALLY_BOUND_BIT (Phase 1
    // Device-Feature) erlaubt unbenutzte Slots im Descriptor-Set. Ein
    // freigegebener Slot wird beim naechsten allocate() einfach ueberschrieben.
}

rhi::RHIResult<void> BindlessTextureHeap::update(uint32_t index, rhi::TextureHandle newTexture) {
    rhi::BindGroupEntry entry;
    entry.binding = 0;
    entry.arrayElement = index;
    entry.texture = newTexture;

    return m_backend->updateBindGroup(m_bindGroup, std::span<const rhi::BindGroupEntry>(&entry, 1));
}

} // namespace axiom::renderer
