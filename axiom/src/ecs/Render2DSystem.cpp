#include "axiom/ecs/Render2DSystem.h"
#include "axiom/ecs/Entity.h"
#include "axiom/ecs/Scene.h"
//#include "axiom/renderer/DrawCommandBuffer.h"
//#include "axiom/renderer/Renderer.h"
//#include "axiom/renderer/Renderer2D.h"

#include <glm/gtc/matrix_inverse.hpp>

namespace axiom {

    Render2DSystem::Render2DSystem(uint32_t viewportWidth, uint32_t viewportHeight)
        : m_ViewportWidth(viewportWidth), m_ViewportHeight(viewportHeight) {
    }

    void Render2DSystem::SetViewport(uint32_t width, uint32_t height) {
        m_ViewportWidth = width;
        m_ViewportHeight = height;
    }

    void Render2DSystem::OnViewportResize(uint32_t width, uint32_t height) {
        SetViewport(width, height);
    }

    void Render2DSystem::BeginRenderFrame() {
        m_HasRenderedFrame = false;
    }

    void Render2DSystem::Render(Scene& scene, double alpha) {
        (void)alpha;

        /*Entity cameraEntity = scene.GetPrimaryCameraEntity();
        if (!cameraEntity)
            return;

        const auto& cameraTransform = cameraEntity.GetComponent<TransformComponent>();
        const auto& camera = cameraEntity.GetComponent<CameraComponent>();
        const float aspectRatio = m_ViewportHeight > 0
            ? static_cast<float>(m_ViewportWidth) / static_cast<float>(m_ViewportHeight)
            : 1.0f;

        const glm::mat4 viewProjection = camera.GetProjection(aspectRatio) * glm::inverse(cameraTransform.GetTransform());

        ClearState clearState;
        clearState.ClearColor = !m_HasRenderedFrame;
        clearState.ClearDepth = !m_HasRenderedFrame;

        Renderer::BeginScene(viewProjection, clearState);
        m_HasRenderedFrame = true;
        Renderer2D::BeginScene();

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
        Renderer::EndScene();*/
    }

} // namespace axiom
