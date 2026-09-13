#pragma once

#include <axiom/renderer/RenderItem.h>
#include <axiom/renderer/RenderLayerRegistry.h>
#include <span>
#include <vector>

namespace axiom::renderer {

using PassID = uint32_t;

enum class SortMode {
    None,
    PipelineGrouped,
    FrontToBack,
    BackToFront,
};

struct RenderQueueDescriptor {
    RenderLayerMask acceptedLayers = ~RenderLayerMask{0};
    SortMode sortMode = SortMode::None;
};

namespace RenderSort {
    void sort(std::vector<RenderItem>& items, SortMode mode);
}

class RenderQueueSystem {
public:
    [[nodiscard]] std::vector<std::vector<RenderItem>> build(
        std::span<const RenderItem> itemPool,
        std::span<const RenderQueueDescriptor> passDescriptors,
        RenderLayerMask visibleLayers = ~RenderLayerMask{0}) const;
};

struct DrawBatch {
    std::shared_ptr<Material> material;
    rhi::BufferHandle vertexBuffer;
    rhi::BufferHandle indexBuffer;
    uint32_t indexCount = 0;
    std::vector<glm::mat4> instanceTransforms;
};

namespace BatchBuilder {
    [[nodiscard]] std::vector<DrawBatch> build(std::span<const RenderItem> sortedItems);
}

} // namespace axiom::renderer
