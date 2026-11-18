/**
 * @file axiom/include/axiom/renderer/RenderItem.h
 * @brief Declares the public interface for the render item.
 */

#pragma once
#include <cstdint>
#include <memory>
#include <glm/glm.hpp>
#include <axiom/renderer/rhi/Handle.h>
#include <axiom/renderer/Material.h>
#include <axiom/renderer/RenderLayerRegistry.h>

namespace axiom::renderer {

// Wird aus ECS-Components gebaut und roh (unsortiert) an den Renderer
// uebergeben (siehe Design-Doc Abschnitt 8/10). Sortierung/Batching sind
// Renderer-Sache, nicht ECS-Sache - RenderSubmissionSystem-Ableitungen
// fuellen nur die Felder hier, mehr nicht.
/** @brief Stores the data for the render item. */
struct RenderItem {
    /** @brief Stores the layer. */
    RenderLayerMask layer = 0;
    /** @brief Stores the z index. */
    int32_t zIndex = 0;       // relevant fuer 2D/UI
    /** @brief Stores the depth. */
    float depth = 0.0f;       // relevant fuer 3D-Transparenz (View-Space-Tiefe)
    /** @brief Stores the material. */
    std::shared_ptr<Material> material;
    /** @brief Stores the vertex buffer. */
    rhi::BufferHandle vertexBuffer;
    /** @brief Stores the index buffer. */
    rhi::BufferHandle indexBuffer;
    /** @brief Stores the index count. */
    uint32_t indexCount = 0;
    /** @brief Stores the transform. */
    glm::mat4 transform{1.0f};
    /** @brief Stores the color. */
    glm::vec4 color{1.0f};
};

} // namespace axiom::renderer
