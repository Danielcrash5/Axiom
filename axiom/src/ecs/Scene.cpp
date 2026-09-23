#include "axiom/ecs/Scene.h"
#include "axiom/ecs/Entity.h"
// #include "axiom/renderer/DrawCommandBuffer.h"
// #include "axiom/renderer/Renderer2D.h"

namespace axiom {
    Entity Scene::CreateEntity(const std::string &name) {
        // Bewusst anonyme flecs-Entity (kein flecs-eigener Name) - die
        // "Namensverwaltung" laeuft weiterhin komplett ueber TagComponent,
        // wie zuvor mit entt, um nicht zwei parallele Namenssysteme zu haben.
        flecs::entity handle = m_World.entity();
        Entity entity(handle, this);
        entity.AddComponent<IDComponent>();
        entity.AddComponent<TagComponent>(name.empty() ? "Entity" : name);
        entity.AddComponent<TransformComponent>();
        return entity;
    }

    void Scene::DestroyEntity(Entity entity) {
        entity.m_Entity.destruct();
    }

    std::vector<std::shared_ptr<Entity>> Scene::GetAllEntities() {
        std::vector<std::shared_ptr<Entity>> entities;
        auto query = m_World.query<TransformComponent>();
        query.each([&](flecs::entity e, TransformComponent &) {
            entities.push_back(std::make_shared<Entity>(e, this));
        });
        return entities;
    }

    void Scene::Render2D() {
        /*DrawCommandBuffer2D commandBuffer;

        auto spriteView = m_Registry.view<TransformComponent,
        SpriteRendererComponent>(); for (auto entity : spriteView) { auto&
        transform = spriteView.get<TransformComponent>(entity); auto&
        spriteRenderer = spriteView.get<SpriteRendererComponent>(entity);

                if (spriteRenderer.HasTexture()) {
                        commandBuffer.SubmitSprite(transform.GetTransform(),
        spriteRenderer.SpriteData, spriteRenderer.Color); continue;
                }

                commandBuffer.SubmitQuad(transform.GetTransform(),
        spriteRenderer.Color);
        }

        auto circleView = m_Registry.view<TransformComponent,
        CircleRendererComponent>(); for (auto entity : circleView) { auto&
        transform = circleView.get<TransformComponent>(entity); auto&
        circleRenderer = circleView.get<CircleRendererComponent>(entity);
                commandBuffer.SubmitCircle(transform.GetTransform(),
        circleRenderer.Thickness, circleRenderer.Color);
        }

        commandBuffer.Sort();
        commandBuffer.Execute();*/
    }

    Entity Scene::GetPrimaryCameraEntity() {
        auto query = m_World.query<TransformComponent, CameraComponent>();

        Entity result{};
        bool found = false;
        query.each([&](flecs::entity e, TransformComponent &, CameraComponent &camera) {
            if (found) return;
            if (camera.Primary) {
                result = Entity(e, this);
                found = true;
            }
        });
        if (found) return result;

        // Kein Primary-Camera markiert - erste gefundene Kamera als Fallback,
        // wie zuvor mit entt.
        query.each([&](flecs::entity e, TransformComponent &, CameraComponent &) {
            if (!found) {
                result = Entity(e, this);
                found = true;
            }
        });
        return result;
    }
} // namespace axiom
