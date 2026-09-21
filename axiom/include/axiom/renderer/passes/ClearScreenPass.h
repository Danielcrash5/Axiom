#pragma once
#include <axiom/renderer/rendergraph/RenderPass.h>
#include <axiom/renderer/View.h>

namespace axiom::renderer::passes {

    class ClearScreenPass final : public rendergraph::RenderPass {
      public:
        void setup(rendergraph::RenderGraphBuilder &builder) override {
            // Wenn diese View ein Swapchain-Image fuer diesen Frame hat
            // (presentTarget), DIREKT dort reinclearen und importieren -
            // sonst landet der Clear in einer eigenen Offscreen-Textur, die
            // nie praesentiert wird und das Fenster bleibt schwarz. Fallback
            // (eigene Transient-Textur) bleibt fuer Views mit
            // RenderTargetKind::Texture oder isolierte Unit-Tests ohne View.
            if (rhi::TextureHandle present = builder.presentTarget(); present.valid()) {
                rendergraph::TextureResourceDesc presentDesc{
                    .width = builder.currentView() ? builder.currentView()->viewport.width : 0,
                    .height = builder.currentView() ? builder.currentView()->viewport.height : 0,
                    .format = rhi::TextureFormat::BGRA8Unorm,
                    // CopyDst (nicht RenderTarget) - requiredLayoutFor() mappt
                    // Write sonst auf ColorAttachment, aber clearTexture()
                    // (Vulkan) erwartet TransferDst-Layout fuer vkCmdClearColorImage.
                    .usage = rhi::TextureUsage::CopyDst,
                    .debugName = "SwapchainPresentTarget",
                };
                m_target = builder.write(builder.importTexture(present, presentDesc));
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
