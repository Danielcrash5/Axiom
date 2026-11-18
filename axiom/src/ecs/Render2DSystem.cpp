/**
 * @file axiom/src/ecs/Render2DSystem.cpp
 * @brief Implements the render2 d system module.
 */

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
        : m_renderer(&renderer), m_viewId(viewId),
          m_viewportWidth(viewportWidth), m_viewportHeight(viewportHeight) {}

    void Render2DSystem::setViewport(uint32_t width, uint32_t height) {
        m_viewportWidth = width;
        m_viewportHeight = height;
    }

    void Render2DSystem::onViewportResize(uint32_t width, uint32_t height) {
        setViewport(width, height);
    }

    void Render2DSystem::render(Scene &scene, double alpha) {
        (void)alpha;
        if (!m_renderer || m_viewId == renderer::kInvalidViewID) {
            return;
        }

        Entity cameraEntity = scene.getPrimaryCameraEntity();
        if (!cameraEntity) {
            return;
        }

        const auto *currentView = m_renderer->view(m_viewId);
        if (!currentView) {
            return;
        }

        const uint32_t width = currentView->viewport.width > 0
                                   ? currentView->viewport.width
                                   : m_viewportWidth;
        const uint32_t height = currentView->viewport.height > 0
                                    ? currentView->viewport.height
                                    : m_viewportHeight;
        const float aspectRatio =
            height > 0
                ? static_cast<float>(width) / static_cast<float>(height)
                : 1.0f;

        const auto &cameraTransform =
            cameraEntity.getComponent<TransformComponent>();
        const auto &camera = cameraEntity.getComponent<CameraComponent>();
        renderer::View updatedView = *currentView;
        updatedView.viewMatrix = glm::inverse(cameraTransform.getTransform());
        updatedView.projectionMatrix = camera.getProjection(aspectRatio);
        m_renderer->updateView(m_viewId, updatedView);

        int32_t zIndex = 0;
        auto quadView = scene.view<TransformComponent, QuadRendererComponent>();
        quadView.each([&](flecs::entity, TransformComponent &transform,
                          QuadRendererComponent &quad) {
            renderer::RenderItem item;
            item.layer = 1u;
            item.zIndex = zIndex++;
            item.depth = transform.Translation.z;
            item.transform = transform.getTransform();
            item.color = quad.Color;
            m_renderer->submitItem(std::move(item));
        });
    }

} // namespace axiom
