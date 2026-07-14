#pragma once
#include "RenderContext.h"
#include "RenderGraphBuilder.h"
#include <axiom/renderer/rhi/CommandList.h>

namespace axiom::renderer::rendergraph {

class RenderPass {
  public:
    virtual ~RenderPass() = default;
    virtual void setup(RenderGraphBuilder &builder) = 0;
    virtual void execute(RenderContext &ctx, rhi::CommandList &cmd) = 0;

    // Rein informativ (Debug/Profiling), keine Logik daran geknüpft.
    [[nodiscard]] virtual const char *name() const = 0;
};

} // namespace axiom::renderer::rendergraph