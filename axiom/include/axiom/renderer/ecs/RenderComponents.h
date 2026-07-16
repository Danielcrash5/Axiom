#pragma once
#include <memory>
#include <axiom/renderer/Material.h>
#include <axiom/renderer/RenderLayerRegistry.h>

namespace axiom::renderer {

// Wird im Editor zu einer Entity hinzugefuegt -> automatische Assoziation
// beim Rendern (siehe Design-Doc Abschnitt 8). Kein manueller
// Registrierungsschritt noetig - GeometrySubmissionSystem<T> prueft einfach
// ueber Entity::HasComponent<MaterialComponent>().
struct MaterialComponent {
    std::shared_ptr<Material> material;
};

// Optional - fehlt sie, greift der "Default"-Layer der RenderLayerRegistry
// (Bit 0, siehe RenderLayerRegistry). Manuell gesetzt, um Entities gezielt
// eigenen Post-Effekten zuzuordnen.
struct RenderLayerComponent {
    RenderLayerMask mask = 0;
};

} // namespace axiom::renderer
