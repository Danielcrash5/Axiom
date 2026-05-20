#include "axiom/ecs/Render2DSystem.h"
#include "axiom/ecs/Scene.h"
#include "axiom/renderer/Renderer2D.h"

namespace axiom {

    void Render2DSystem::Render(Scene& scene, double alpha) {
        (void)alpha;

        auto spriteView = scene.View<TransformComponent, SpriteRendererComponent>();
        spriteView.each([&](auto entity, TransformComponent& transform, SpriteRendererComponent& spriteRenderer) {
            if (spriteRenderer.HasTexture()) {
                Renderer2D::DrawSprite(transform.GetTransform(), spriteRenderer.SpriteData, spriteRenderer.Color);
                return;
            }

            Renderer2D::DrawQuad(transform.GetTransform(), spriteRenderer.Color);
        });

        auto circleView = scene.View<TransformComponent, CircleRendererComponent>();
        circleView.each([&](auto entity, TransformComponent& transform, CircleRendererComponent& circleRenderer) {
            Renderer2D::DrawCircle(transform.GetTransform(), circleRenderer.Thickness, circleRenderer.Color);
        });

        Renderer2D::EndScene();
    }

} // namespace axiom
