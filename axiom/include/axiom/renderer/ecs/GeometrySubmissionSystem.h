/**
 * @file axiom/include/axiom/renderer/ecs/GeometrySubmissionSystem.h
 * @brief Declares the public interface for the geometry submission system.
 */

#pragma once
#include <memory>

#include <axiom/ecs/ISystem.h>
#include <axiom/ecs/Scene.h>
#include <axiom/ecs/Entity.h>
#include <axiom/ecs/Components.h>

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
/**
 * @brief Processes the geometry submission system system work.
 * @tparam GeometryComponent Template type used by this type.
 * @tparam GeometrySubmissionSystem Template type used by this type.
 */
template <typename GeometryComponent>
class GeometrySubmissionSystem : public axiom::ISystem {
public:
    /**
     * @brief Creates an instance of geometry submission system.
     * @param renderer The renderer used by this operation.
     * @param layerRegistry The layer registry used by this operation.
     */
    GeometrySubmissionSystem(Renderer& renderer, RenderLayerRegistry& layerRegistry)
        : m_renderer(renderer), m_layerRegistry(layerRegistry) {}

    /**
     * @brief Performs the render operation.
     * @param scene The scene used by this operation.
     */
    void Render(axiom::Scene& scene, double /*alpha*/) override {
        auto query = scene.View<axiom::TransformComponent, GeometryComponent>();
        query.each([&](flecs::entity entityHandle, axiom::TransformComponent& transform,
                        GeometryComponent& geometry) {
            RenderItem item;
            item.transform = transform.GetTransform();

            if (!buildItem(geometry, item)) {
                return; // Ableitung signalisiert "nicht renderbar" (statt continue - Lambda-Body)
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
        });
    }

protected:
    // Pro Geometrie-Typ ueberschrieben (Phase 6): fuellt vertexBuffer/
    // indexBuffer/indexCount/zIndex/depth aus der Component. false =
    // Item wird nicht submitted (z.B. Component markiert unsichtbar).
    /**
     * @brief Builds item.
     * @param geometry The geometry used by this operation.
     * @param[out] outItem Receives the item value.
     * @return The result of the operation.
     */
    virtual bool buildItem(const GeometryComponent& geometry, RenderItem& outItem) = 0;

    // Default-Material, falls die Entity kein MaterialComponent hat.
    /**
     * @brief Performs the default material for operation.
     * @param geometry The geometry used by this operation.
     * @return The result of the operation.
     */
    [[nodiscard]] virtual std::shared_ptr<Material> defaultMaterialFor(
        const GeometryComponent& geometry) const = 0;

private:
    /** @brief Stores the renderer. */
    Renderer& m_renderer;
    /** @brief Stores the layer registry. */
    RenderLayerRegistry& m_layerRegistry;
};

} // namespace axiom::renderer
