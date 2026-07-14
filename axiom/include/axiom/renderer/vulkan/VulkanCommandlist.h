#pragma once
#include <vulkan/vulkan.h>
#include <axiom/renderer/rhi/CommandList.h>

namespace axiom::renderer::rhi::vulkan {

class VulkanBackend; // fwd

class VulkanCommandList final : public CommandList {
public:
    VulkanCommandList(VulkanBackend& backend, VkCommandBuffer commandBuffer)
        : m_backend(backend), m_commandBuffer(commandBuffer) {}

    void copyBufferToBuffer(BufferHandle src, uint64_t srcOffset,
                             BufferHandle dst, uint64_t dstOffset,
                             uint64_t sizeBytes) override;
    void transitionTexture(TextureHandle texture,
                            TextureLayout oldLayout, TextureLayout newLayout) override;
    void clearTexture(TextureHandle texture, ClearColor color) override;

    [[nodiscard]] VkCommandBuffer nativeHandle() const { return m_commandBuffer; }
    void bindPipeline(PipelineHandle pipelineHandle);

private:
    VulkanBackend& m_backend;
    VkCommandBuffer m_commandBuffer;
};

} // namespace axiom::renderer::rhi::vulkan