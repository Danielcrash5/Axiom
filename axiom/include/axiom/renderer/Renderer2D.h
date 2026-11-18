/**
 * @file axiom/include/axiom/renderer/Renderer2D.h
 * @brief Declares the public interface for the renderer2 d.
 */

#pragma once

#include <axiom/renderer/RenderItem.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <span>
#include <vector>

namespace axiom::renderer {

class Renderer;

/**
 * @brief Builds and submits two-dimensional render items.
 * @tparam Renderer2D Template type used by this type.
 */
class Renderer2D {
public:
    /**
     * @brief Sets active renderer.
     * @param renderer The renderer used by this operation.
     */
    static void setActiveRenderer(Renderer* renderer);
    /**
     * @brief Performs the active renderer operation.
     * @return The result of the operation.
     */
    static Renderer* activeRenderer();

    /**
     * @brief Begins scene.
     * @param viewProjection The view projection used by this operation.
     */
    static void beginScene(const glm::mat4& viewProjection = glm::mat4(1.0f));
    /**
     * @brief Ends scene.
     */
    static void endScene();
    /**
     * @brief Performs the flush operation.
     */
    static void flush();

    /**
     * @brief Submits quad.
     * @param transform The transform used by this operation.
     * @param color The color used by this operation.
     * @param tilingFactor The tiling factor used by this operation.
     */
    static void submitQuad(const glm::mat4& transform, const glm::vec4& color,
                           float tilingFactor = 1.0f);
    /**
     * @brief Submits circle.
     * @param transform The transform used by this operation.
     * @param thickness The thickness used by this operation.
     * @param color The color used by this operation.
     */
    static void submitCircle(const glm::mat4& transform, float thickness = 1.0f,
                             const glm::vec4& color = glm::vec4(1.0f));
    /**
     * @brief Submits line.
     * @param start The start used by this operation.
     * @param end The end used by this operation.
     * @param color The color used by this operation.
     */
    static void submitLine(const glm::vec3& start, const glm::vec3& end,
                           const glm::vec4& color = glm::vec4(1.0f));
    /**
     * @brief Submits rect.
     * @param position The position used by this operation.
     * @param size The size used by this operation.
     * @param color The color used by this operation.
     */
    static void submitRect(const glm::vec3& position, const glm::vec2& size,
                           const glm::vec4& color);

    /**
     * @brief Draws quad.
     * @param position The position used by this operation.
     * @param size The size used by this operation.
     * @param color The color used by this operation.
     */
    static void drawQuad(const glm::vec2& position, const glm::vec2& size,
                         const glm::vec4& color);
    /**
     * @brief Draws rotated quad.
     * @param position The position used by this operation.
     * @param size The size used by this operation.
     * @param rotationRadians The rotation radians used by this operation.
     * @param color The color used by this operation.
     */
    static void drawRotatedQuad(const glm::vec2& position, const glm::vec2& size,
                                float rotationRadians,
                                const glm::vec4& color);
    /**
     * @brief Draws circle.
     * @param position The position used by this operation.
     * @param radius The radius used by this operation.
     * @param color The color used by this operation.
     * @param thickness The thickness used by this operation.
     */
    static void drawCircle(const glm::vec2& position, float radius,
                           const glm::vec4& color,
                           float thickness = 1.0f);
    /**
     * @brief Draws line.
     * @param start The start used by this operation.
     * @param end The end used by this operation.
     * @param color The color used by this operation.
     */
    static void drawLine(const glm::vec2& start, const glm::vec2& end,
                         const glm::vec4& color);

    /**
     * @brief Performs the queued items operation.
     * @return The result of the operation.
     */
    static std::span<const RenderItem> queuedItems();
    /**
     * @brief Performs the queued item count operation.
     * @return The result of the operation.
     */
    static size_t queuedItemCount();
    /**
     * @brief Clears queued items.
     */
    static void clearQueuedItems();

private:
    /** @brief Stores the data for the scene data. */
    struct SceneData {
        /** @brief Stores the view projection. */
        glm::mat4 viewProjection{1.0f};
        /** @brief Stores the active. */
        bool active = false;
        /** @brief Stores the queued items. */
        std::vector<RenderItem> queuedItems;
        /** @brief Stores the next z index. */
        int32_t nextZIndex = 0;
    };

    /**
     * @brief Performs the scene operation.
     * @return The result of the operation.
     */
    static SceneData& scene();
    /**
     * @brief Adds item.
     * @param item The item used by this operation.
     */
    static void pushItem(RenderItem item);
    /** @brief Stores the renderer. */
    static Renderer* sRenderer;
};

} // namespace axiom::renderer
