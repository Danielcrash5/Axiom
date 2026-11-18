/**
 * @file axiom/include/axiom/renderer/RenderQueue.h
 * @brief Declares the public interface for the render queue.
 */

#pragma once

#include <axiom/renderer/RenderItem.h>
#include <axiom/renderer/RenderLayerRegistry.h>
#include <span>
#include <vector>

namespace axiom::renderer {

/**
 * @brief Provides the pass ID type alias.
 */
using PassID = uint32_t;

/**
 * @brief Defines the available sort mode values.
 * @tparam SortMode Template type used by this type.
 */
enum class SortMode {
    /** @brief Represents the none value. */
    none,
    /** @brief Represents the pipeline grouped value. */
    pipelineGrouped,
    /** @brief Represents the front to back value. */
    frontToBack,
    /** @brief Represents the back to front value. */
    backToFront,
};

/** @brief Stores the data for the render queue descriptor. */
struct RenderQueueDescriptor {
    /** @brief Stores the accepted layers. */
    RenderLayerMask acceptedLayers = ~RenderLayerMask{0};
    /** @brief Stores the sort mode. */
    SortMode sortMode = SortMode::none;
};

namespace render_sort {
    /**
     * @brief Sorts the stored items.
     * @param items The items used by this operation.
     * @param mode The mode used by this operation.
     */
    void sort(std::vector<RenderItem>& items, SortMode mode);
}

/**
 * @brief Processes the render queue system system work.
 * @tparam RenderQueueSystem Template type used by this type.
 */
class RenderQueueSystem {
public:
    /**
     * @brief Builds the requested resource.
     * @param itemPool The item pool used by this operation.
     * @param passDescriptors The pass descriptors used by this operation.
     * @param visibleLayers The visible layers used by this operation.
     * @return The result of the operation.
     */
    [[nodiscard]] std::vector<std::vector<RenderItem>> build(
        std::span<const RenderItem> itemPool,
        std::span<const RenderQueueDescriptor> passDescriptors,
        RenderLayerMask visibleLayers = ~RenderLayerMask{0}) const;
};

/** @brief Stores the data for the draw batch. */
struct DrawBatch {
    /** @brief Stores the material. */
    std::shared_ptr<Material> material;
    /** @brief Stores the vertex buffer. */
    rhi::BufferHandle vertexBuffer;
    /** @brief Stores the index buffer. */
    rhi::BufferHandle indexBuffer;
    /** @brief Stores the index count. */
    uint32_t indexCount = 0;
    /** @brief Stores the instance transforms. */
    std::vector<glm::mat4> instanceTransforms;
};

namespace batch_builder {
    /**
     * @brief Builds the requested resource.
     * @param sortedItems The sorted items used by this operation.
     * @return The result of the operation.
     */
    [[nodiscard]] std::vector<DrawBatch> build(std::span<const RenderItem> sortedItems);
}

} // namespace axiom::renderer
