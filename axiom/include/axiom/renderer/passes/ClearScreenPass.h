#pragma once
#include <axiom/renderer/rendergraph/RenderPass.h>
#include <axiom/renderer/View.h>

namespace axiom::renderer::passes {

    class ClearScreenPass final : public rendergraph::RenderPass {
      public:
        void setup(rendergraph::RenderGraphBuilder &builder) override {
            // Groesse jetzt aus der aktuellen View (falls vorhanden) statt
            // hartkodiert - RenderGraphBuilder kennt seit der Swapchain-
            // Anbindung dieselbe RenderExecutionDesc wie RenderContext.
            // Fallback 800x600 fuer Aufrufer ohne View (z.B. Unit-Tests, die
            // den Graph isoliert kompilieren).
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
