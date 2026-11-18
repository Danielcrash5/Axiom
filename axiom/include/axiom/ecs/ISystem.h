/**
 * @file axiom/include/axiom/ecs/ISystem.h
 * @brief Declares the public interface for the i system.
 */

#pragma once

#include <cstdint>

namespace axiom {

    class Scene;

    /**
     * @brief Defines the interface for the system.
     * @tparam ISystem Template type used by this type.
     */
    class ISystem {
      public:
        /**
         * @brief Releases resources owned by ISystem.
         */
        virtual ~ISystem() = default;

        /**
         * @brief Updates state before the main update step.
         * @param scene The scene used by this operation.
         * @param dt The dt used by this operation.
         */
        virtual void preUpdate(Scene &scene, double dt) {}
        /**
         * @brief Updates the object state.
         * @param scene The scene used by this operation.
         * @param dt The dt used by this operation.
         */
        virtual void update(Scene &scene, double dt) {}
        /**
         * @brief Updates state at a fixed time interval.
         * @param scene The scene used by this operation.
         * @param dt The dt used by this operation.
         */
        virtual void fixedUpdate(Scene &scene, double dt) {}
        /**
         * @brief Performs work after the main update step.
         * @param scene The scene used by this operation.
         * @param dt The dt used by this operation.
         */
        virtual void postUpdate(Scene &scene, double dt) {}
        /**
         * @brief Begins render frame.
         */
        virtual void beginRenderFrame() {}
        /**
         * @brief Renders the current frame.
         * @param scene The scene used by this operation.
         * @param alpha The alpha used by this operation.
         */
        virtual void render(Scene &scene, double alpha) {}
        /**
         * @brief Handles the viewport resize callback.
         * @param width The width used by this operation.
         * @param height The height used by this operation.
         */
        virtual void onViewportResize(uint32_t width, uint32_t height) {}
    };

} // namespace axiom
