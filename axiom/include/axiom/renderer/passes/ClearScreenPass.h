#pragma once
#include <axiom/renderer/rendergraph/RenderPass.h>

namespace axiom::renderer::passes {

class ClearScreenPass final : public rendergraph::RenderPass {
  public:
    void setup(rendergraph::RenderGraphBuilder &builder) override {
        rendergraph::TextureResourceDesc desc{
            .width = 800,
            .height = 600,
            .format = rhi::TextureFormat::RGBA8Unorm,
            .usage = rhi::TextureUsage::CopyDst | rhi::TextureUsage::Sampled,
            .debugName = "ClearTestTarget",
        };
        m_target = builder.write(builder.createTexture(desc));
    }

    void execute(rendergraph::RenderContext &ctx,
                 rhi::CommandList &cmd) override {
        auto handle = ctx.resolveTexture(m_target);
        cmd.clearTexture(handle, rhi::ClearColor{0.05f, 0.05f, 0.08f, 1.0f});
    }

    [[nodiscard]] const char *name() const override {
        return "ClearScreenPass";
    }

  private:
    rendergraph::ResourceHandle m_target;
};

} // namespace axiom::renderer::passes
#include <axiom/renderer/rendergraph/RenderPass.h>

namespace axiom::renderer::passes {

class ClearScreenPass final : public rendergraph::RenderPass {
  public:
    void setup(rendergraph::RenderGraphBuilder &builder) override {
        rendergraph::TextureResourceDesc desc{
            .width = 800,
            .height = 600,
            .format = rhi::TextureFormat::RGBA8Unorm,
            .usage = rhi::TextureUsage::CopyDst | rhi::TextureUsage::Sampled,
            .debugName = "ClearTestTarget",
        };
        m_target = builder.write(builder.createTexture(desc));
    }

    void execute(rendergraph::RenderContext &ctx,
                 rhi::CommandList &cmd) override {
        auto handle = ctx.resolveTexture(m_target);
        cmd.clearTexture(handle, rhi::ClearColor{0.05f, 0.05f, 0.08f, 1.0f});
    }

    [[nodiscard]] const char *name() const override {
        return "ClearScreenPass";
    }

  private:
    rendergraph::ResourceHandle m_target;
};

} // namespace axiom::renderer::passes