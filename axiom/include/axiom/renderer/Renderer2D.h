#pragma once

#include <axiom/renderer/RenderItem.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <span>
#include <vector>

namespace axiom::renderer {

class Renderer;

class Renderer2D {
public:
    static void setActiveRenderer(Renderer* renderer);
    static Renderer* activeRenderer();

    static void BeginScene(const glm::mat4& viewProjection = glm::mat4(1.0f));
    static void EndScene();
    static void Flush();

    static void SubmitQuad(const glm::mat4& transform, const glm::vec4& color,
                           float tilingFactor = 1.0f);
    static void SubmitCircle(const glm::mat4& transform, float thickness = 1.0f,
                             const glm::vec4& color = glm::vec4(1.0f));
    static void SubmitLine(const glm::vec3& start, const glm::vec3& end,
                           const glm::vec4& color = glm::vec4(1.0f));
    static void SubmitRect(const glm::vec3& position, const glm::vec2& size,
                           const glm::vec4& color);

    static void DrawQuad(const glm::vec2& position, const glm::vec2& size,
                         const glm::vec4& color);
    static void DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size,
                                float rotationRadians,
                                const glm::vec4& color);
    static void DrawCircle(const glm::vec2& position, float radius,
                           const glm::vec4& color,
                           float thickness = 1.0f);
    static void DrawLine(const glm::vec2& start, const glm::vec2& end,
                         const glm::vec4& color);

    static std::span<const RenderItem> queuedItems();
    static size_t queuedItemCount();
    static void clearQueuedItems();

    // Compatibility aliases for the older lowercase style used in some code.
    static void beginScene(const glm::mat4& viewProjection = glm::mat4(1.0f));
    static void endScene();
    static void flush();
    static void submitQuad(const glm::mat4& transform, const glm::vec4& color,
                           float tilingFactor = 1.0f);
    static void submitCircle(const glm::mat4& transform, float thickness = 1.0f,
                             const glm::vec4& color = glm::vec4(1.0f));
    static void submitLine(const glm::vec3& start, const glm::vec3& end,
                           const glm::vec4& color = glm::vec4(1.0f));

private:
    struct SceneData {
        glm::mat4 viewProjection{1.0f};
        bool active = false;
        std::vector<RenderItem> queuedItems;
        int32_t nextZIndex = 0;
    };

    static SceneData& scene();
    static void pushItem(RenderItem item);
    static Renderer* s_renderer;
};

} // namespace axiom::renderer
