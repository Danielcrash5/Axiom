#include <axiom/renderer/BindlessTextureHeap.h>

namespace axiom::renderer {

rhi::RHIResult<void> BindlessTextureHeap::init(rhi::IRHIBackend& backend) {
    shutdown();
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
    m_slots.assign(kMaxTextures, {});
    m_slotUsed.assign(kMaxTextures, false);
    m_freeSlots.clear();
    m_nextSlot = 0;

    return {};
}

void BindlessTextureHeap::shutdown() {
    if (m_backend) {
        if (m_bindGroup.valid()) {
            m_backend->destroyBindGroup(m_bindGroup);
        }
        if (m_layout.valid()) {
            m_backend->destroyBindGroupLayout(m_layout);
        }
    }

    m_backend = nullptr;
    m_layout = {};
    m_bindGroup = {};
    m_slots.clear();
    m_slotUsed.clear();
    m_freeSlots.clear();
    m_nextSlot = 0;
}

rhi::RHIResult<uint32_t> BindlessTextureHeap::allocate(rhi::TextureHandle texture) {
    if (!initialized() || !texture.valid()) {
        return std::unexpected(rhi::RHIError::InvalidDescriptor);
    }

    uint32_t slot;
    bool reusedSlot = false;
    if (!m_freeSlots.empty()) {
        slot = m_freeSlots.back();
        m_freeSlots.pop_back();
        reusedSlot = true;
    } else {
        if (m_nextSlot >= kMaxTextures) {
            return std::unexpected(rhi::RHIError::OutOfMemory);
        }
        slot = m_nextSlot++;
    }

    if (auto result = update(slot, texture); !result) {
        if (reusedSlot) {
            m_freeSlots.push_back(slot);
        } else {
            --m_nextSlot;
        }
        return std::unexpected(result.error());
    }
    m_slots[slot] = texture;
    m_slotUsed[slot] = true;
    return slot;
}

void BindlessTextureHeap::free(uint32_t index) {
    if (index >= m_slotUsed.size() || !m_slotUsed[index]) {
        return;
    }

    m_slotUsed[index] = false;
    m_slots[index] = {};
    m_freeSlots.push_back(index);
    // Bewusst KEIN Descriptor-Reset auf "leer" - PARTIALLY_BOUND_BIT (Phase 1
    // Device-Feature) erlaubt unbenutzte Slots im Descriptor-Set. Ein
    // freigegebener Slot wird beim naechsten allocate() einfach ueberschrieben.
}

rhi::RHIResult<void> BindlessTextureHeap::update(uint32_t index, rhi::TextureHandle newTexture) {
    if (!initialized() || index >= kMaxTextures || !newTexture.valid()) {
        return std::unexpected(rhi::RHIError::InvalidDescriptor);
    }

    rhi::BindGroupEntry entry;
    entry.binding = 0;
    entry.arrayElement = index;
    entry.texture = newTexture;

    auto result =
        m_backend->updateBindGroup(m_bindGroup,
                                   std::span<const rhi::BindGroupEntry>(&entry, 1));
    if (result && index < m_slots.size()) {
        m_slots[index] = newTexture;
    }
    return result;
}

bool BindlessTextureHeap::isAllocated(uint32_t index) const {
    return index < m_slotUsed.size() && m_slotUsed[index];
}

} // namespace axiom::renderer
