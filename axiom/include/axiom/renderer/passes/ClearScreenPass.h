#pragma once
#include <axiom/renderer/rendergraph/RenderPass.h>
#include <axiom/renderer/View.h>

namespace axiom::renderer::passes {

    class ClearScreenPass final : public rendergraph::RenderPass {
      public:
        void setup(rendergraph::RenderGraphBuilder &builder) override {
            // Wenn diese View ein Swapchain-Image fuer diesen Frame hat,
            // DIREKT dort reinclearen - sonst landet der Clear in einer
            // eigenen Offscreen-Textur, die nie praesentiert wird und das
            // Fenster bleibt schwarz. Das Present-Target kommt vom Graph
            // (einmal zentral importiert); das Layout gibt dieser Zugriff
            // selbst an: clearTexture() (Vulkan) erwartet TransferDst fuer
            // vkCmdClearColorImage. Fallback (eigene Transient-Textur)
            // bleibt fuer Views mit RenderTargetKind::Texture oder
            // isolierte Unit-Tests ohne View.
            if (const auto present = builder.presentTargetResource();
                present.valid()) {
                m_target = builder.write(present, rhi::TextureLayout::TransferDst);
                return;
            }

            uint32_t width = 800, height = 600;
            if (const View *view = builder.currentView()) {
                if (view->viewport.width > 0 && view->viewport.height > 0) {
                    width = view->viewport.width;
                    height = view->viewport.height;
                }
            }

            rendergraph::TextureResourceDesc desc{
                .width = width,
                .height = height,
                .format = rhi::TextureFormat::RGBA8Unorm,
                .usage =
                    rhi::TextureUsage::CopyDst | rhi::TextureUsage::Sampled,
                .debugName = "ClearTestTarget",
            };
            m_target = builder.write(builder.createTexture(desc));
        }

        void execute(rendergraph::RenderContext &ctx,
                     rhi::CommandList &cmd) override {
            auto handle = ctx.resolveTexture(m_target);
            cmd.clearTexture(handle,
                             rhi::ClearColor{0.05f, 0.05f, 0.08f, 1.0f});
        }

        [[nodiscard]] const char *name() const override {
            return "ClearScreenPass";
        }

      private:
        rendergraph::ResourceHandle m_target;
    };

} // namespace axiom::renderer::passes
