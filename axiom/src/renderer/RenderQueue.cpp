#include <axiom/renderer/RenderQueue.h>
#include <algorithm>
#include <cstdint>
#include <tuple>

namespace axiom::renderer {

namespace {

uint64_t shaderKey(const RenderItem& item) {
    if (!item.material) {
        return 0;
    }
    return static_cast<uint64_t>(item.material->shaderId);
}

uintptr_t materialKey(const RenderItem& item) {
    return reinterpret_cast<uintptr_t>(item.material.get());
}

auto pipelineTuple(const RenderItem& item) {
    return std::tuple{
        shaderKey(item),
        materialKey(item),
        item.vertexBuffer.index,
        item.vertexBuffer.generation,
        item.indexBuffer.index,
        item.indexBuffer.generation,
        item.indexCount,
    };
}

bool sameBatchKey(const RenderItem& left, const RenderItem& right) {
    return left.material == right.material &&
           left.vertexBuffer == right.vertexBuffer &&
           left.indexBuffer == right.indexBuffer &&
           left.indexCount == right.indexCount;
}

} // namespace

void RenderSort::sort(std::vector<RenderItem>& items, SortMode mode) {
    switch (mode) {
    case SortMode::None:
        return;
    case SortMode::PipelineGrouped:
        std::stable_sort(items.begin(), items.end(), [](const RenderItem& left,
                                                        const RenderItem& right) {
            return pipelineTuple(left) < pipelineTuple(right);
        });
        return;
    case SortMode::FrontToBack:
        std::stable_sort(items.begin(), items.end(), [](const RenderItem& left,
                                                        const RenderItem& right) {
            return std::tuple{left.layer, left.depth, left.zIndex, pipelineTuple(left)} <
                   std::tuple{right.layer, right.depth, right.zIndex, pipelineTuple(right)};
        });
        return;
    case SortMode::BackToFront:
        std::stable_sort(items.begin(), items.end(), [](const RenderItem& left,
                                                        const RenderItem& right) {
            return std::tuple{left.layer, -left.depth, left.zIndex, pipelineTuple(left)} <
                   std::tuple{right.layer, -right.depth, right.zIndex, pipelineTuple(right)};
        });
        return;
    }
}

std::vector<std::vector<RenderItem>> RenderQueueSystem::build(
    std::span<const RenderItem> itemPool,
    std::span<const RenderQueueDescriptor> passDescriptors,
    RenderLayerMask visibleLayers) const {
    std::vector<std::vector<RenderItem>> queues(passDescriptors.size());

    for (size_t passIndex = 0; passIndex < passDescriptors.size(); ++passIndex) {
        const RenderQueueDescriptor& descriptor = passDescriptors[passIndex];
        const RenderLayerMask acceptedVisibleLayers =
            descriptor.acceptedLayers & visibleLayers;

        if (acceptedVisibleLayers == 0) {
            continue;
        }

        auto& queue = queues[passIndex];
        for (const RenderItem& item : itemPool) {
            if ((item.layer & acceptedVisibleLayers) != 0) {
                queue.push_back(item);
            }
        }

        RenderSort::sort(queue, descriptor.sortMode);
    }

    return queues;
}

std::vector<DrawBatch> BatchBuilder::build(std::span<const RenderItem> sortedItems) {
    std::vector<DrawBatch> batches;
    const RenderItem* previousItem = nullptr;

    for (const RenderItem& item : sortedItems) {
        if (item.indexCount == 0 || !item.vertexBuffer.valid() ||
            !item.indexBuffer.valid()) {
            continue;
        }

        if (!previousItem || !sameBatchKey(item, *previousItem)) {
            DrawBatch batch;
            batch.material = item.material;
            batch.vertexBuffer = item.vertexBuffer;
            batch.indexBuffer = item.indexBuffer;
            batch.indexCount = item.indexCount;
            batches.push_back(std::move(batch));
        }

        batches.back().instanceTransforms.push_back(item.transform);
        previousItem = &item;
    }

    return batches;
}

} // namespace axiom::renderer
