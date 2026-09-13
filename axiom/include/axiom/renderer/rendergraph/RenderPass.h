#pragma once
#include "RenderContext.h"
#include "RenderGraphBuilder.h"
#include <axiom/renderer/RenderQueue.h>
#include <axiom/renderer/rhi/CommandList.h>

namespace axiom::renderer::rendergraph {

    class RenderPass {
      public:
        virtual ~RenderPass() = default;
        virtual void setup(RenderGraphBuilder &builder) = 0;
        virtual void execute(RenderContext &ctx, rhi::CommandList &cmd) = 0;

        // Rein informativ (Debug/Profiling), keine Logik daran geknüpft.
        [[nodiscard]] virtual const char *name() const = 0;

        // Item-basierte Passes koennen hier ihre Queue-Anforderungen melden.
        // Post-FX/Clear/Compute-Passes lassen den Default stehen und bekommen
        // keine RenderItems vorbereitet.
        [[nodiscard]] virtual bool usesRenderQueue() const { return false; }
        [[nodiscard]] virtual RenderQueueDescriptor queueDescriptor() const {
            return {};
        }
    };

} // namespace axiom::renderer::rendergraph
