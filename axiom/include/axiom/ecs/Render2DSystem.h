/**
 * @file axiom/include/axiom/ecs/Render2DSystem.h
 * @brief Declares the public interface for the render2 d system.
 */

#pragma once

#include "ISystem.h"
#include <axiom/renderer/View.h>

namespace axiom {

    namespace renderer {
        class Renderer;
    }

    /**
     * @brief Processes the render2 d system system work.
     * @tparam Render2DSystem Template type used by this type.
     */
    class Render2DSystem : public ISystem {
      public:
        /**
         * @brief Creates an instance of render2 d system.
         * @param renderer The renderer used by this operation.
         * @param viewId The view ID used by this operation.
         * @param viewportWidth The viewport width used by this operation.
         * @param viewportHeight The viewport height used by this operation.
         */
        Render2DSystem(renderer::Renderer &renderer, renderer::ViewID viewId,
                       uint32_t viewportWidth = 1280,
                       uint32_t viewportHeight = 720);

        /**
         * @brief Sets viewport.
         * @param width The width used by this operation.
         * @param height The height used by this operation.
         */
        void setViewport(uint32_t width, uint32_t height);
        /**
         * @brief Handles the viewport resize callback.
         * @param width The width used by this operation.
         * @param height The height used by this operation.
         */
        void onViewportResize(uint32_t width, uint32_t height) override;
        /**
         * @brief Renders the current frame.
         * @param scene The scene used by this operation.
         * @param alpha The alpha used by this operation.
         */
        void render(Scene &scene, double alpha) override;

      private:
        /** @brief Stores the renderer. */
        renderer::Renderer *m_renderer = nullptr;
        /** @brief Stores the view ID. */
        renderer::ViewID m_viewId = renderer::kInvalidViewID;
        /** @brief Stores the viewport width. */
        uint32_t m_viewportWidth = 1280;
        /** @brief Stores the viewport height. */
        uint32_t m_viewportHeight = 720;
    };

} // namespace axiom
