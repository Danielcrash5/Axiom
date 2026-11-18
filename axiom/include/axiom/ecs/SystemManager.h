/**
 * @file axiom/include/axiom/ecs/SystemManager.h
 * @brief Declares the public interface for the system manager.
 */

#pragma once

#include "ISystem.h"

#include <algorithm>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

namespace axiom {

    class Scene;

    /**
     * @brief Coordinates the systems registered for scene updates and rendering.
     * @tparam SystemManager Template type used by this type.
     */
    class SystemManager {
      public:
        /**
         * @brief Adds system.
         * @tparam T Template type used by this operation.
         * @param args The args used by this operation.
         * @return The result of the operation.
         */
        template <typename T, typename... Args>
        std::shared_ptr<T> addSystem(Args &&...args) {
            static_assert(std::is_base_of_v<ISystem, T>,
                          "System must derive from axiom::ISystem");
            auto system = std::make_shared<T>(std::forward<Args>(args)...);
            m_systems.push_back(system);
            return system;
        }

        /**
         * @brief Removes system.
         * @param system The system used by this operation.
         */
        void removeSystem(const std::shared_ptr<ISystem> &system) {
            m_systems.erase(
                std::remove(m_systems.begin(), m_systems.end(), system),
                m_systems.end());
        }

        /**
         * @brief Clears the stored state.
         */
        void clear() { m_systems.clear(); }

        /**
         * @brief Updates state before the main update step.
         * @param scene The scene used by this operation.
         * @param dt The dt used by this operation.
         */
        void preUpdate(Scene &scene, double dt) {
            for (auto &system : m_systems) {
                if (system)
                    system->preUpdate(scene, dt);
            }
        }

        /**
         * @brief Updates the object state.
         * @param scene The scene used by this operation.
         * @param dt The dt used by this operation.
         */
        void update(Scene &scene, double dt) {
            for (auto &system : m_systems) {
                if (system)
                    system->update(scene, dt);
            }
        }

        /**
         * @brief Updates state at a fixed time interval.
         * @param scene The scene used by this operation.
         * @param dt The dt used by this operation.
         */
        void fixedUpdate(Scene &scene, double dt) {
            for (auto &system : m_systems) {
                if (system)
                    system->fixedUpdate(scene, dt);
            }
        }

        /**
         * @brief Performs work after the main update step.
         * @param scene The scene used by this operation.
         * @param dt The dt used by this operation.
         */
        void postUpdate(Scene &scene, double dt) {
            for (auto &system : m_systems) {
                if (system)
                    system->postUpdate(scene, dt);
            }
        }

        /**
         * @brief Renders the current frame.
         * @param scene The scene used by this operation.
         * @param alpha The alpha used by this operation.
         */
        void render(Scene &scene, double alpha) {
            for (auto &system : m_systems) {
                if (system)
                    system->render(scene, alpha);
            }
        }

        /**
         * @brief Begins render frame.
         */
        void beginRenderFrame() {
            for (auto &system : m_systems) {
                if (system)
                    system->beginRenderFrame();
            }
        }

        /**
         * @brief Handles the viewport resize callback.
         * @param width The width used by this operation.
         * @param height The height used by this operation.
         */
        void onViewportResize(uint32_t width, uint32_t height) {
            for (auto &system : m_systems) {
                if (system)
                    system->onViewportResize(width, height);
            }
        }

      private:
        /** @brief Stores the systems. */
        std::vector<std::shared_ptr<ISystem>> m_systems;
    };

} // namespace axiom
