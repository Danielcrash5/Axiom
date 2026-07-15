#pragma once
#include <cstdint>
#include <vector>
#include <axiom/renderer/rhi/IRHIBackend.h>

namespace axiom::renderer {

// Vulkan Descriptor Indexing als Primaerpfad (Design-Doc Abschnitt 6.1) -
// Device-Features (descriptorIndexing, runtimeDescriptorArray,
// partiallyBound) wurden schon in Phase 1 aktiviert. Kein Vulkan-Header
// hier - laeuft komplett ueber IRHIBackend::updateBindGroup.
class BindlessTextureHeap {
public:
    static constexpr uint32_t kMaxTextures = 4096;

    [[nodiscard]] rhi::RHIResult<void> init(rhi::IRHIBackend& backend);

    [[nodiscard]] rhi::RHIResult<uint32_t> allocate(rhi::TextureHandle texture);
    void free(uint32_t index);
    [[nodiscard]] rhi::RHIResult<void> update(uint32_t index, rhi::TextureHandle newTexture);

    [[nodiscard]] rhi::BindGroupLayoutHandle layout() const { return m_layout; }
    [[nodiscard]] rhi::BindGroupHandle bindGroup() const { return m_bindGroup; }

private:
    rhi::IRHIBackend* m_backend = nullptr;
    rhi::BindGroupLayoutHandle m_layout;
    rhi::BindGroupHandle m_bindGroup;
    std::vector<uint32_t> m_freeSlots;
    uint32_t m_nextSlot = 0;
};

} // namespace axiom::renderer
