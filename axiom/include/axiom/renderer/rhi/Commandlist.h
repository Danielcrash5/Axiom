/**
 * @file axiom/include/axiom/renderer/rhi/Commandlist.h
 * @brief Declares the public interface for the commandlist.
 */

#pragma once
#include <cstdint>
#include <optional>
#include "Handle.h"

namespace axiom::renderer::rhi {

// Layouts abstrahiert genug, dass es nicht 1:1 VkImageLayout ist - ein
// kuenftiges WebGPU-Backend (Web-Export, Phase 11) hat kein Layout-Konzept
// in dieser Form, kann diese Werte aber auf seine Barriers abbilden.
/**
 * @brief Defines the available texture layout values.
 * @tparam TextureLayout Template type used by this type.
 */
enum class TextureLayout {
    /** @brief Represents the undefined value. */
    undefined,
    /** @brief Represents the transfer src value. */
    transferSrc,
    /** @brief Represents the transfer dst value. */
    transferDst,
    /** @brief Represents the color attachment value. */
    colorAttachment,
    /** @brief Represents the shader read only value. */
    shaderReadOnly,
    /** @brief Represents the present value. */
    present,
};

/** @brief Stores the data for the clear color. */
struct ClearColor { float r = 0, g = 0, b = 0, a = 1; };

/**
 * @brief Provides the command list functionality.
 * @tparam CommandList Template type used by this type.
 */
class CommandList {
public:
    /**
     * @brief Releases resources owned by CommandList.
     */
    virtual ~CommandList() = default;

    // --- Phase 1 ---
    /**
     * @brief Copies a range of bytes between buffers.
     * @param src The src used by this operation.
     * @param srcOffset The src offset used by this operation.
     * @param dst The dst used by this operation.
     * @param dstOffset The dst offset used by this operation.
     * @param sizeBytes The size bytes used by this operation.
     */
    virtual void copyBufferToBuffer(BufferHandle src, uint64_t srcOffset,
                                     BufferHandle dst, uint64_t dstOffset,
                                     uint64_t sizeBytes) = 0;

    // --- Phase 2 ---
    /**
     * @brief Transitions a texture to the requested layout.
     * @param texture The texture used by this operation.
     * @param oldLayout The old layout used by this operation.
     * @param newLayout The new layout used by this operation.
     */
    virtual void transitionTexture(TextureHandle texture,
                                    TextureLayout oldLayout, TextureLayout newLayout) = 0;
    /**
     * @brief Clears texture.
     * @param texture The texture used by this operation.
     * @param color The color used by this operation.
     */
    virtual void clearTexture(TextureHandle texture, ClearColor color) = 0;

    // --- Phase 3 ---
    /**
     * @brief Binds pipeline.
     */
    virtual void bindPipeline(PipelineHandle) = 0;
    /**
     * @brief Binds vertex buffer.
     * @param slot The slot used by this operation.
     */
    virtual void bindVertexBuffer(uint32_t slot, BufferHandle) = 0;
    /**
     * @brief Binds index buffer.
     * @param use16Bit The use16 bit used by this operation.
     */
    virtual void bindIndexBuffer(BufferHandle, bool use16Bit) = 0; // true=Uint16, false=Uint32
    /**
     * @brief Binds group.
     * @param set The set used by this operation.
     */
    virtual void bindGroup(uint32_t set, BindGroupHandle) = 0;
    /**
     * @brief Draws the requested geometry.
     * @param vertexCount The vertex count used by this operation.
     * @param instanceCount The instance count used by this operation.
     * @param firstVertex The first vertex used by this operation.
     * @param firstInstance The first instance used by this operation.
     */
    virtual void draw(uint32_t vertexCount, uint32_t instanceCount,
                       uint32_t firstVertex, uint32_t firstInstance) = 0;
    /**
     * @brief Draws indexed.
     * @param indexCount The index count used by this operation.
     * @param instanceCount The instance count used by this operation.
     * @param firstIndex The first index used by this operation.
     */
    virtual void drawIndexed(uint32_t indexCount, uint32_t instanceCount,
                              uint32_t firstIndex) = 0;
    /**
     * @brief Dispatches the pending event or command.
     * @param x The x used by this operation.
     * @param y The y used by this operation.
     * @param z The z used by this operation.
     */
    virtual void dispatch(uint32_t x, uint32_t y, uint32_t z) = 0;

    // Fuer Dynamic Rendering: Pass muss Color/Depth-Attachments selbst
    // "beginnen"/"beenden" (Ersatz fuer VkRenderPass::begin).
    // clearColor gesetzt -> loadOp = CLEAR, sonst LOAD (Inhalt bleibt erhalten).
    /**
     * @brief Begins rendering.
     * @param colorTarget The color target used by this operation.
     * @param depthTarget The depth target used by this operation.
     * @param clearColor The clear color used by this operation.
     */
    virtual void beginRendering(TextureHandle colorTarget,
                                 std::optional<TextureHandle> depthTarget,
                                 std::optional<ClearColor> clearColor = std::nullopt) = 0;
    /**
     * @brief Ends rendering.
     */
    virtual void endRendering() = 0;
};

} // namespace axiom::renderer::rhi
