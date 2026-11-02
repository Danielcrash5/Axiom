#include "axiom/ecs/Render2DSystem.h"
#include "axiom/ecs/Entity.h"
#include "axiom/ecs/Scene.h"
#include "axiom/renderer/Renderer.h"

#include <glm/gtc/matrix_inverse.hpp>

namespace axiom {

    Render2DSystem::Render2DSystem(renderer::Renderer &renderer,
                                   renderer::ViewID viewId,
                                   uint32_t viewportWidth,
                                   uint32_t viewportHeight)
        : m_Renderer(&renderer), m_ViewId(viewId),
          m_ViewportWidth(viewportWidth), m_ViewportHeight(viewportHeight) {}

    void Render2DSystem::SetViewport(uint32_t width, uint32_t height) {
        m_ViewportWidth = width;
        m_ViewportHeight = height;
    }

    void Render2DSystem::OnViewportResize(uint32_t width, uint32_t height) {
        SetViewport(width, height);
    }

    void Render2DSystem::Render(Scene &scene, double alpha) {
        (void)alpha;
        if (!m_Renderer || m_ViewId == renderer::kInvalidViewID) {
            return;
        }

        Entity cameraEntity = scene.GetPrimaryCameraEntity();
        if (!cameraEntity) {
            return;
        }

        const auto *currentView = m_Renderer->view(m_ViewId);
        if (!currentView) {
            return;
        }

        const uint32_t width = currentView->viewport.width > 0
                                   ? currentView->viewport.width
                                   : m_ViewportWidth;
        const uint32_t height = currentView->viewport.height > 0
                                    ? currentView->viewport.height
                                    : m_ViewportHeight;
        const float aspectRatio =
            height > 0
                ? static_cast<float>(width) / static_cast<float>(height)
                : 1.0f;

        const auto &cameraTransform =
            cameraEntity.GetComponent<TransformComponent>();
        const auto &camera = cameraEntity.GetComponent<CameraComponent>();
        renderer::View updatedView = *currentView;
        updatedView.viewMatrix = glm::inverse(cameraTransform.GetTransform());
        updatedView.projectionMatrix = camera.GetProjection(aspectRatio);
        m_Renderer->updateView(m_ViewId, updatedView);

        int32_t zIndex = 0;
        auto quadView = scene.View<TransformComponent, QuadRendererComponent>();
        quadView.each([&](flecs::entity, TransformComponent &transform,
                          QuadRendererComponent &quad) {
            renderer::RenderItem item;
            item.layer = 1u;
            item.zIndex = zIndex++;
            item.depth = transform.Translation.z;
            item.transform = transform.GetTransform();
            item.color = quad.Color;
            m_Renderer->submitItem(std::move(item));
        });
    }

} // namespace axiom
