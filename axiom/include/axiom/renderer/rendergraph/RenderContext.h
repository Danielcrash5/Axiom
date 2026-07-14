#pragma once
#include <axiom/renderer/rhi/RHITypes.h>
#include "ResourceHandle.h"

namespace axiom::renderer::rendergraph {

class RenderGraph;

// Wird an RenderPass::execute() gereicht – Passes lösen ihre in setup()
// deklarierten ResourceHandles hier auf echte rhi::TextureHandle auf.
class RenderContext {
public:
    explicit RenderContext(RenderGraph& graph) : m_graph(graph) {}

    [[nodiscard]] rhi::TextureHandle resolveTexture(ResourceHandle handle) const;

private:
    RenderGraph& m_graph;
};

} // namespace axiom::renderer::rendergraph