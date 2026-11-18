/**
 * @file axiom/include/axiom/renderer/ecs/RenderComponents.h
 * @brief Declares the public interface for the render components.
 */

#pragma once
#include <memory>
#include <axiom/renderer/Material.h>
#include <axiom/renderer/RenderLayerRegistry.h>

namespace axiom::renderer {

// Wird im Editor zu einer Entity hinzugefuegt -> automatische Assoziation
// beim Rendern (siehe Design-Doc Abschnitt 8). Kein manueller
// Registrierungsschritt noetig - GeometrySubmissionSystem<T> prueft einfach
// ueber Entity::HasComponent<MaterialComponent>().
/** @brief Stores the data for the material component component. */
struct MaterialComponent {
    /** @brief Stores the material. */
    std::shared_ptr<Material> material;
};

// Optional - fehlt sie, greift der "Default"-Layer der RenderLayerRegistry
// (Bit 0, siehe RenderLayerRegistry). Manuell gesetzt, um Entities gezielt
// eigenen Post-Effekten zuzuordnen.
/** @brief Stores the data for the render layer component component. */
struct RenderLayerComponent {
    /** @brief Stores the mask. */
    RenderLayerMask mask = 0;
};

} // namespace axiom::renderer
