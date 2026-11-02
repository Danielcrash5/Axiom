#pragma once

#include <axiom/renderer/rendergraph/RenderPass.h>
#include <axiom/renderer/rhi/IRHIBackend.h>

namespace axiom::renderer {

class Renderer;

namespace passes {

class QuadRenderPass final : public rendergraph::RenderPass {
public:
    QuadRenderPass() = default;
    ~QuadRenderPass() override;

    QuadRenderPass(const QuadRenderPass&) = delete;
    QuadRenderPass& operator=(const QuadRenderPass&) = delete;

    [[nodiscard]] rhi::RHIResult<void> initialize(
        Renderer& renderer, rhi::SurfaceHandle surface,
        rhi::TextureFormat colorTargetFormat);

    void setup(rendergraph::RenderGraphBuilder& builder) override;
    void execute(rendergraph::RenderContext& ctx, rhi::CommandList& cmd) override;
    [[nodiscard]] const char* name() const override { return "QuadRenderPass"; }

private:
    rhi::IRHIBackend* m_backend = nullptr;
    rhi::SurfaceHandle m_surface;
    rhi::BufferHandle m_vertexBuffer;
    rhi::BufferHandle m_indexBuffer;
    rhi::PipelineHandle m_pipeline;
    rendergraph::ResourceHandle m_target;
};

} // namespace passes
} // namespace axiom::renderer
