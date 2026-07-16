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
struct RenderItem {
    RenderLayerMask layer = 0;
    int32_t zIndex = 0;       // relevant fuer 2D/UI
    float depth = 0.0f;       // relevant fuer 3D-Transparenz (View-Space-Tiefe)
    std::shared_ptr<Material> material;
    rhi::BufferHandle vertexBuffer;
    rhi::BufferHandle indexBuffer;
    uint32_t indexCount = 0;
    glm::mat4 transform{1.0f};
};

} // namespace axiom::renderer
