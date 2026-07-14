#pragma once
#include <axiom/renderer/rhi/IRHIBackend.h>
#include <vector>

namespace axiom::renderer {

    // Vulkan Descriptor Indexing als Primärpfad (Abschnitt 6.1) –
    // Device-Features (descriptorIndexing, runtimeDescriptorArray,
    // partiallyBound) wurden schon in Phase 1 aktiviert.
    class BindlessTextureHeap {
      public:
        static constexpr uint32_t kMaxTextures = 4096;

        [[nodiscard]] rhi::RHIResult<void> init(rhi::IRHIBackend &backend);

        [[nodiscard]] uint32_t allocate(rhi::TextureHandle texture);
        void free(uint32_t index);
        void update(uint32_t index, rhi::TextureHandle newTexture);

        [[nodiscard]] rhi::BindGroupLayoutHandle layout() const {
            return m_layout;
        }
        [[nodiscard]] rhi::BindGroupHandle bindGroup() const {
            return m_bindGroup;
        }

      private:
        rhi::IRHIBackend *m_backend = nullptr;
        rhi::BindGroupLayoutHandle m_layout;
        rhi::BindGroupHandle m_bindGroup;
        std::vector<uint32_t> m_freeSlots;
        uint32_t m_nextSlot = 0;
    };

} // namespace axiom::renderer