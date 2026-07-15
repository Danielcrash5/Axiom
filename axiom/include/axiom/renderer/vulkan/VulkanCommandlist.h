#pragma once
#include <vulkan/vulkan.h>
#include <optional>
#include <axiom/renderer/rhi/CommandList.h>

namespace axiom::renderer::rhi::vulkan {

class VulkanBackend; // fwd

class VulkanCommandList final : public CommandList {
public:
    VulkanCommandList(VulkanBackend& backend, VkCommandBuffer commandBuffer)
        : m_backend(backend), m_commandBuffer(commandBuffer) {}

    // --- Phase 1 ---
    void copyBufferToBuffer(BufferHandle src, uint64_t srcOffset,
                             BufferHandle dst, uint64_t dstOffset,
                             uint64_t sizeBytes) override;

    // --- Phase 2 ---
    void transitionTexture(TextureHandle texture,
                            TextureLayout oldLayout, TextureLayout newLayout) override;
    void clearTexture(TextureHandle texture, ClearColor color) override;

    // --- Phase 3 ---
    void bindPipeline(PipelineHandle) override;
    void bindVertexBuffer(uint32_t slot, BufferHandle) override;
    void bindIndexBuffer(BufferHandle, bool use16Bit) override;
    void bindGroup(uint32_t set, BindGroupHandle) override;
    void draw(uint32_t vertexCount, uint32_t instanceCount,
              uint32_t firstVertex, uint32_t firstInstance) override;
    void drawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex) override;
    void dispatch(uint32_t x, uint32_t y, uint32_t z) override;
    void beginRendering(TextureHandle colorTarget,
                         std::optional<TextureHandle> depthTarget) override;
    void endRendering() override;

    [[nodiscard]] VkCommandBuffer nativeHandle() const { return m_commandBuffer; }

private:
    VulkanBackend& m_backend;
    VkCommandBuffer m_commandBuffer;
    VkPipelineLayout m_currentPipelineLayout = VK_NULL_HANDLE; // fuer bindGroup() gebraucht
};

} // namespace axiom::renderer::rhi::vulkan
