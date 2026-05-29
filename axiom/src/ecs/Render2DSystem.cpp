#include "axiom/ecs/Render2DSystem.h"
#include "axiom/ecs/Scene.h"
#include "axiom/renderer/DrawCommandBuffer.h"
#include "axiom/renderer/Renderer2D.h"

namespace axiom {

    void Render2DSystem::Render(Scene& scene, double alpha) {
        (void)alpha;

        DrawCommandBuffer2D commandBuffer;

        auto spriteView = scene.View<TransformComponent, SpriteRendererComponent>();
        spriteView.each([&](auto entity, TransformComponent& transform, SpriteRendererComponent& spriteRenderer) {
            (void)entity;

            if (spriteRenderer.HasTexture()) {
                commandBuffer.SubmitSprite(transform.GetTransform(), spriteRenderer.SpriteData, spriteRenderer.Color);
                return;
            }

            commandBuffer.SubmitQuad(transform.GetTransform(), spriteRenderer.Color);
        });

        auto circleView = scene.View<TransformComponent, CircleRendererComponent>();
        circleView.each([&](auto entity, TransformComponent& transform, CircleRendererComponent& circleRenderer) {
            (void)entity;
            commandBuffer.SubmitCircle(transform.GetTransform(), circleRenderer.Thickness, circleRenderer.Color);
        });

        commandBuffer.Sort();
        commandBuffer.Execute();
        Renderer2D::EndScene();
    }

} // namespace axiom
