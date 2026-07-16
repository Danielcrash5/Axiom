#include <axiom/renderer/vulkan/VulkanBackend.h>
#include <axiom/renderer/vulkan/VulkanCommandlist.h>

namespace axiom::renderer::rhi::vulkan {

namespace {

VkImageLayout toVkImageLayout(TextureLayout layout) {
    switch (layout) {
        case TextureLayout::Undefined:       return VK_IMAGE_LAYOUT_UNDEFINED;
        case TextureLayout::TransferSrc:     return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        case TextureLayout::TransferDst:     return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        case TextureLayout::ColorAttachment: return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        case TextureLayout::ShaderReadOnly:  return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        case TextureLayout::Present:         return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    }
    return VK_IMAGE_LAYOUT_UNDEFINED;
}

// Grobe, konservative Stage/Access-Masken pro Layout - reicht fuer Phase 2/3.
// Feinere, pass-spezifische Barriers sind eine spaetere Optimierung.
void stageAccessForLayout(VkImageLayout layout, VkPipelineStageFlags2& stage, VkAccessFlags2& access) {
    switch (layout) {
        case VK_IMAGE_LAYOUT_UNDEFINED:
            stage = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT; access = 0; return;
        case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
            stage = VK_PIPELINE_STAGE_2_TRANSFER_BIT; access = VK_ACCESS_2_TRANSFER_WRITE_BIT; return;
        case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
            stage = VK_PIPELINE_STAGE_2_TRANSFER_BIT; access = VK_ACCESS_2_TRANSFER_READ_BIT; return;
        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
            stage = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
            access = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT; return;
        case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
            stage = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT; access = VK_ACCESS_2_SHADER_READ_BIT; return;
        case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
            stage = VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT; access = 0; return;
        default:
            stage = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
            access = VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT; return;
    }
}

} // anonymous namespace

// --- Phase 1 ---

void VulkanCommandList::copyBufferToBuffer(BufferHandle src, uint64_t srcOffset,
                                            BufferHandle dst, uint64_t dstOffset,
                                            uint64_t sizeBytes) {
    VkBuffer srcBuffer = m_backend.nativeBuffer(src);
    VkBuffer dstBuffer = m_backend.nativeBuffer(dst);
    if (srcBuffer == VK_NULL_HANDLE || dstBuffer == VK_NULL_HANDLE) return;

    VkBufferCopy region{};
    region.srcOffset = srcOffset;
    region.dstOffset = dstOffset;
    region.size = sizeBytes;
    vkCmdCopyBuffer(m_commandBuffer, srcBuffer, dstBuffer, 1, &region);
}

// --- Phase 2 ---

void VulkanCommandList::transitionTexture(TextureHandle texture,
                                           TextureLayout oldLayout, TextureLayout newLayout) {
    VkImage image = m_backend.nativeImage(texture);
    if (image == VK_NULL_HANDLE) return;

    VkImageLayout vkOld = toVkImageLayout(oldLayout);
    VkImageLayout vkNew = toVkImageLayout(newLayout);

    VkPipelineStageFlags2 srcStage, dstStage;
    VkAccessFlags2 srcAccess, dstAccess;
    stageAccessForLayout(vkOld, srcStage, srcAccess);
    stageAccessForLayout(vkNew, dstStage, dstAccess);

    VkImageMemoryBarrier2 barrier{ .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2 };
    barrier.srcStageMask = srcStage;
    barrier.srcAccessMask = srcAccess;
    barrier.dstStageMask = dstStage;
    barrier.dstAccessMask = dstAccess;
    barrier.oldLayout = vkOld;
    barrier.newLayout = vkNew;
    barrier.image = image;
    barrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };

    VkDependencyInfo depInfo{ .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO };
    depInfo.imageMemoryBarrierCount = 1;
    depInfo.pImageMemoryBarriers = &barrier;
    vkCmdPipelineBarrier2(m_commandBuffer, &depInfo);
}

void VulkanCommandList::clearTexture(TextureHandle texture, ClearColor color) {
    VkImage image = m_backend.nativeImage(texture);
    if (image == VK_NULL_HANDLE) return;

    VkClearColorValue clearValue{};
    clearValue.float32[0] = color.r;
    clearValue.float32[1] = color.g;
    clearValue.float32[2] = color.b;
    clearValue.float32[3] = color.a;

    VkImageSubresourceRange range{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
    // Erwartet TRANSFER_DST_OPTIMAL - RenderGraph::execute() stellt das ueber
    // requiredLayoutFor(AccessType::Write) + transitionTexture() vorher sicher.
    vkCmdClearColorImage(m_commandBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                          &clearValue, 1, &range);
}

// --- Phase 3 ---

void VulkanCommandList::bindPipeline(PipelineHandle handle) {
    auto native = m_backend.nativePipeline(handle);
    if (native.pipeline == VK_NULL_HANDLE) return;
    m_currentPipelineLayout = native.layout;
    vkCmdBindPipeline(m_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, native.pipeline);
}

void VulkanCommandList::bindVertexBuffer(uint32_t slot, BufferHandle handle) {
    VkBuffer buffer = m_backend.nativeBuffer(handle);
    if (buffer == VK_NULL_HANDLE) return;
    VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers(m_commandBuffer, slot, 1, &buffer, &offset);
}

void VulkanCommandList::bindIndexBuffer(BufferHandle handle, bool use16Bit) {
    VkBuffer buffer = m_backend.nativeBuffer(handle);
    if (buffer == VK_NULL_HANDLE) return;
    vkCmdBindIndexBuffer(m_commandBuffer, buffer, 0,
                          use16Bit ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32);
}

void VulkanCommandList::bindGroup(uint32_t set, BindGroupHandle handle) {
    VkDescriptorSet descriptorSet = m_backend.nativeDescriptorSet(handle);
    if (descriptorSet == VK_NULL_HANDLE || m_currentPipelineLayout == VK_NULL_HANDLE) return;
    vkCmdBindDescriptorSets(m_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                             m_currentPipelineLayout, set, 1, &descriptorSet, 0, nullptr);
}

void VulkanCommandList::draw(uint32_t vertexCount, uint32_t instanceCount,
                              uint32_t firstVertex, uint32_t firstInstance) {
    vkCmdDraw(m_commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
}

void VulkanCommandList::drawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex) {
    vkCmdDrawIndexed(m_commandBuffer, indexCount, instanceCount, firstIndex, 0, 0);
}

void VulkanCommandList::dispatch(uint32_t x, uint32_t y, uint32_t z) {
    vkCmdDispatch(m_commandBuffer, x, y, z);
}

void VulkanCommandList::beginRendering(TextureHandle colorTarget,
                                        std::optional<TextureHandle> depthTarget) {
    VkImageView colorView = m_backend.nativeImageView(colorTarget);
    auto [width, height] = m_backend.nativeExtent(colorTarget);

    VkRenderingAttachmentInfo colorAttachment{ .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO };
    colorAttachment.imageView = colorView;
    colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD; // RenderGraph hat schon geclearet falls noetig
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

    VkRenderingInfo renderingInfo{ .sType = VK_STRUCTURE_TYPE_RENDERING_INFO };
    renderingInfo.renderArea = { {0, 0}, {width, height} };
    renderingInfo.layerCount = 1;
    renderingInfo.colorAttachmentCount = 1;
    renderingInfo.pColorAttachments = &colorAttachment;

    VkRenderingAttachmentInfo depthAttachment{ .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO };
    if (depthTarget) {
        depthAttachment.imageView = m_backend.nativeImageView(*depthTarget);
        depthAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
        depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
        depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        renderingInfo.pDepthAttachment = &depthAttachment;
    }

    vkCmdBeginRendering(m_commandBuffer, &renderingInfo);

    // Viewport/Scissor hier setzen (dynamic state), inkl. Y-Flip fuer
    // rechtshaendiges Y-up-Koordinatensystem (siehe Design-Doc Abschnitt 1).
    VkViewport viewport{ 0.0f, static_cast<float>(height), static_cast<float>(width),
                          -static_cast<float>(height), 0.0f, 1.0f };
    VkRect2D scissor{ {0, 0}, {width, height} };
    vkCmdSetViewport(m_commandBuffer, 0, 1, &viewport);
    vkCmdSetScissor(m_commandBuffer, 0, 1, &scissor);
}

void VulkanCommandList::endRendering() {
    vkCmdEndRendering(m_commandBuffer);
}

} // namespace axiom::renderer::rhi::vulkan
