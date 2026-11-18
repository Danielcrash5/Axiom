/**
 * @file axiom/include/axiom/renderer/vulkan/VulkanCommandlist.h
 * @brief Declares the public interface for the vulkan commandlist.
 */

#pragma once
#include <vulkan/vulkan.h>
#include <optional>
#include <axiom/renderer/rhi/CommandList.h>

namespace axiom::renderer::rhi::vulkan {

class VulkanBackend; // fwd

/**
 * @brief Provides the vulkan command list functionality.
 * @tparam VulkanCommandList Template type used by this type.
 */
class VulkanCommandList final : public CommandList {
public:
    /**
     * @brief Creates an instance of vulkan command list.
     * @param backend The backend used by this operation.
     * @param commandBuffer The command buffer used by this operation.
     */
    VulkanCommandList(VulkanBackend& backend, VkCommandBuffer commandBuffer)
        : m_backend(backend), m_commandBuffer(commandBuffer) {}

    // --- Phase 1 ---
    /**
     * @brief Copies a range of bytes between buffers.
     * @param src The src used by this operation.
     * @param srcOffset The src offset used by this operation.
     * @param dst The dst used by this operation.
     * @param dstOffset The dst offset used by this operation.
     * @param sizeBytes The size bytes used by this operation.
     */
    void copyBufferToBuffer(BufferHandle src, uint64_t srcOffset,
                             BufferHandle dst, uint64_t dstOffset,
                             uint64_t sizeBytes) override;

    // --- Phase 2 ---
    /**
     * @brief Transitions a texture to the requested layout.
     * @param texture The texture used by this operation.
     * @param oldLayout The old layout used by this operation.
     * @param newLayout The new layout used by this operation.
     */
    void transitionTexture(TextureHandle texture,
                            TextureLayout oldLayout, TextureLayout newLayout) override;
    /**
     * @brief Clears texture.
     * @param texture The texture used by this operation.
     * @param color The color used by this operation.
     */
    void clearTexture(TextureHandle texture, ClearColor color) override;

    // --- Phase 3 ---
    /**
     * @brief Binds pipeline.
     */
    void bindPipeline(PipelineHandle) override;
    /**
     * @brief Binds vertex buffer.
     * @param slot The slot used by this operation.
     */
    void bindVertexBuffer(uint32_t slot, BufferHandle) override;
    /**
     * @brief Binds index buffer.
     * @param use16Bit The use16 bit used by this operation.
     */
    void bindIndexBuffer(BufferHandle, bool use16Bit) override;
    /**
     * @brief Binds group.
     * @param set The set used by this operation.
     */
    void bindGroup(uint32_t set, BindGroupHandle) override;
    /**
     * @brief Draws the requested geometry.
     * @param vertexCount The vertex count used by this operation.
     * @param instanceCount The instance count used by this operation.
     * @param firstVertex The first vertex used by this operation.
     * @param firstInstance The first instance used by this operation.
     */
    void draw(uint32_t vertexCount, uint32_t instanceCount,
              uint32_t firstVertex, uint32_t firstInstance) override;
    /**
     * @brief Draws indexed.
     * @param indexCount The index count used by this operation.
     * @param instanceCount The instance count used by this operation.
     * @param firstIndex The first index used by this operation.
     */
    void drawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex) override;
    /**
     * @brief Dispatches the pending event or command.
     * @param x The x used by this operation.
     * @param y The y used by this operation.
     * @param z The z used by this operation.
     */
    void dispatch(uint32_t x, uint32_t y, uint32_t z) override;
    /**
     * @brief Begins rendering.
     * @param colorTarget The color target used by this operation.
     * @param depthTarget The depth target used by this operation.
     * @param clearColor The clear color used by this operation.
     */
    void beginRendering(TextureHandle colorTarget,
                         std::optional<TextureHandle> depthTarget,
                         std::optional<ClearColor> clearColor) override;
    /**
     * @brief Ends rendering.
     */
    void endRendering() override;

    /**
     * @brief Returns the native handle.
     * @return The result of the operation.
     */
    [[nodiscard]] VkCommandBuffer nativeHandle() const { return m_commandBuffer; }

private:
    /** @brief Stores the backend. */
    VulkanBackend& m_backend;
    /** @brief Stores the command buffer. */
    VkCommandBuffer m_commandBuffer;
    /** @brief Stores the current pipeline layout. */
    VkPipelineLayout m_currentPipelineLayout = VK_NULL_HANDLE; // fuer bindGroup() gebraucht
};

} // namespace axiom::renderer::rhi::vulkan
