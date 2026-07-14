#pragma once
#include "Handle.h"
#include <optional>

namespace axiom::renderer::rhi {

    // Layouts abstrahiert genug, dass es nicht 1:1 VkImageLayout ist – ein
    // künftiges WebGPU-Backend (Web-Export, Phase 11) hat kein Layout-Konzept
    // in dieser Form, kann diese Werte aber auf seine Barriers abbilden.
    enum class TextureLayout {
        Undefined,
        TransferSrc,
        TransferDst,
        ColorAttachment,
        ShaderReadOnly,
        Present,
    };

    struct ClearColor {
        float r = 0, g = 0, b = 0, a = 1;
    };

    class CommandList {
      public:
        virtual ~CommandList() = default;

        virtual void copyBufferToBuffer(BufferHandle src, uint64_t srcOffset,
                                        BufferHandle dst, uint64_t dstOffset,
                                        uint64_t sizeBytes) = 0;

        virtual void transitionTexture(TextureHandle texture,
                                       TextureLayout oldLayout,
                                       TextureLayout newLayout) = 0;
        virtual void clearTexture(TextureHandle texture, ClearColor color) = 0;

        virtual void bindPipeline(PipelineHandle) = 0;
        virtual void bindVertexBuffer(uint32_t slot, BufferHandle) = 0;
        virtual void
        bindIndexBuffer(BufferHandle,
                        bool use16Bit) = 0; // true=Uint16, false=Uint32
        virtual void bindGroup(uint32_t set, BindGroupHandle) = 0;
        virtual void draw(uint32_t vertexCount, uint32_t instanceCount,
                          uint32_t firstVertex, uint32_t firstInstance) = 0;
        virtual void drawIndexed(uint32_t indexCount, uint32_t instanceCount,
                                 uint32_t firstIndex) = 0;
        virtual void dispatch(uint32_t x, uint32_t y, uint32_t z) = 0;

        // Für Dynamic Rendering: Pass muss Color/Depth-Attachments selbst
        // "beginnen"/"beenden" (Ersatz für RenderPass::begin in klassischem
        // Vulkan).
        virtual void
        beginRendering(TextureHandle colorTarget,
                       std::optional<TextureHandle> depthTarget) = 0;
        virtual void endRendering() = 0;
    };

} // namespace axiom::renderer::rhi