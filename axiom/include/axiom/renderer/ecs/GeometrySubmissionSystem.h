#pragma once
#include <memory>

// Pfade angenommen als axiom/scene/... - bitte an eure tatsaechliche
// Ordnerstruktur anpassen, falls abweichend (Scene.h/Entity.h/ISystem.h/
// Components.h wurden nur als lose Dateien hochgeladen, ohne Pfadkontext).
#include <axiom/scene/ISystem.h>
#include <axiom/scene/Scene.h>
#include <axiom/scene/Entity.h>
#include <axiom/scene/Components.h>

#include <axiom/renderer/Renderer.h>
#include <axiom/renderer/RenderItem.h>
#include <axiom/renderer/RenderLayerRegistry.h>
#include <axiom/renderer/ecs/RenderComponents.h>

namespace axiom::renderer {

// Generische Basis fuer "Geometrie-Component -> RenderItem". Kennt selbst
// KEINEN konkreten Geometrie-Typ (Grundprinzip 1: nichts hartkodiert) - jeder
// Geometrie-Typ (Quad, Circle, Line, ... - Phase 6) bekommt seine eigene
// Ableitung, die nur buildItem()/defaultMaterialFor() ausfuellt. Die
// Auto-Association-Logik (Material/RenderLayer, Design-Doc Abschnitt 8) ist
// hier zentral implementiert, nicht pro Ableitung dupliziert.
template <typename GeometryComponent>
class GeometrySubmissionSystem : public axiom::ISystem {
public:
    GeometrySubmissionSystem(Renderer& renderer, RenderLayerRegistry& layerRegistry)
        : m_renderer(renderer), m_layerRegistry(layerRegistry) {}

    void Render(axiom::Scene& scene, double /*alpha*/) override {
        auto view = scene.View<axiom::TransformComponent, GeometryComponent>();
        for (auto&& [entityHandle, transform, geometry] : view.each()) {
            RenderItem item;
            item.transform = transform.GetTransform();

            if (!buildItem(geometry, item)) {
                continue; // Ableitung signalisiert "nicht renderbar"
            }

            axiom::Entity entity(entityHandle, &scene);

            // --- Auto-Association: Material ---
            if (entity.template HasComponent<MaterialComponent>()) {
                item.material = entity.template GetComponent<MaterialComponent>().material;
            } else {
                item.material = defaultMaterialFor(geometry);
            }

            // --- Auto-Association: RenderLayer ---
            if (entity.template HasComponent<RenderLayerComponent>()) {
                item.layer = entity.template GetComponent<RenderLayerComponent>().mask;
            } else {
                item.layer = m_layerRegistry.maskFor(RenderLayerRegistry::kDefaultLayerName);
            }

            m_renderer.submitItem(std::move(item));
        }
    }

protected:
    // Pro Geometrie-Typ ueberschrieben (Phase 6): fuellt vertexBuffer/
    // indexBuffer/indexCount/zIndex/depth aus der Component. false =
    // Item wird nicht submitted (z.B. Component markiert unsichtbar).
    virtual bool buildItem(const GeometryComponent& geometry, RenderItem& outItem) = 0;

    // Default-Material, falls die Entity kein MaterialComponent hat.
    [[nodiscard]] virtual std::shared_ptr<Material> defaultMaterialFor(
        const GeometryComponent& geometry) const = 0;

private:
    Renderer& m_renderer;
    RenderLayerRegistry& m_layerRegistry;
};

} // namespace axiom::renderer
