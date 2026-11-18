/**
 * @file axiom/include/axiom/renderer/passes/QuadRenderPass.h
 * @brief Declares the public interface for the quad render pass.
 */

#pragma once

#include <axiom/renderer/rendergraph/RenderPass.h>
#include <axiom/renderer/rhi/IRHIBackend.h>
#include <cstddef>

namespace axiom::renderer {

class Renderer;

namespace passes {

/**
 * @brief Provides the quad render pass functionality.
 * @tparam QuadRenderPass Template type used by this type.
 */
class QuadRenderPass final : public rendergraph::RenderPass {
public:
    /**
     * @brief Creates an instance of quad render pass.
     */
    QuadRenderPass() = default;
    /**
     * @brief Releases resources owned by QuadRenderPass.
     */
    ~QuadRenderPass() override;

    /**
     * @brief Creates an instance of quad render pass.
     * @param QuadRenderPass The quad render pass used by this operation.
     */
    QuadRenderPass(const QuadRenderPass&) = delete;
    QuadRenderPass& operator=(const QuadRenderPass&) = delete;

    /**
     * @brief Initializes ialize.
     * @param renderer The renderer used by this operation.
     * @param surface The surface used by this operation.
     * @param colorTargetFormat The color target format used by this operation.
     */
    [[nodiscard]] rhi::RHIResult<void> initialize(
        Renderer& renderer, rhi::SurfaceHandle surface,
        rhi::TextureFormat colorTargetFormat);

    /**
     * @brief Configures the resources required by this object.
     * @param builder The builder used by this operation.
     */
    void setup(rendergraph::RenderGraphBuilder& builder) override;
    /**
     * @brief Executes the operation using the supplied context.
     * @param ctx The ctx used by this operation.
     * @param cmd The cmd used by this operation.
     */
    void execute(rendergraph::RenderContext& ctx, rhi::CommandList& cmd) override;
    /**
     * @brief Returns the name.
     * @return The result of the operation.
     */
    [[nodiscard]] const char* name() const override { return "QuadRenderPass"; }
    /**
     * @brief Reports whether this pass uses a render queue.
     * @return The result of the operation.
     */
    [[nodiscard]] bool usesRenderQueue() const override { return true; }
    /**
     * @brief Returns the queue descriptor.
     * @return The result of the operation.
     */
    [[nodiscard]] RenderQueueDescriptor queueDescriptor() const override {
        return {.acceptedLayers = ~RenderLayerMask{0},
                .sortMode = SortMode::none};
    }

private:
    /**
     * @brief Performs the ensure vertex capacity operation.
     * @param vertexCount The vertex count used by this operation.
     */
    [[nodiscard]] rhi::RHIResult<void> ensureVertexCapacity(size_t vertexCount);

    /** @brief Stores the backend. */
    rhi::IRHIBackend* m_backend = nullptr;
    /** @brief Stores the surface. */
    rhi::SurfaceHandle m_surface;
    /** @brief Stores the vertex buffer. */
    rhi::BufferHandle m_vertexBuffer;
    /** @brief Stores the vertex capacity. */
    size_t m_vertexCapacity = 0;
    /** @brief Stores the pipeline. */
    rhi::PipelineHandle m_pipeline;
    /** @brief Stores the target. */
    rendergraph::ResourceHandle m_target;
};

} // namespace passes
} // namespace axiom::renderer
