/**
 * @file axiom/include/axiom/ecs/Scene.h
 * @brief Declares the public interface for the scene.
 */

#pragma once
#include <flecs.h>
#include <memory>
#include <optional>
#include <string>
#include <utility>

#include "Components.h"

namespace axiom {

    class Entity;

    /**
     * @brief Owns the entities and components that make up a scene.
     * @tparam Scene Template type used by this type.
     */
    class Scene {
      public:
        /**
         * @brief Creates an instance of scene.
         * @param name The name used by this operation.
         */
        Scene(const std::string &name = "Scene") : m_name(name) {}

        /**
         * @brief Creates entity.
         * @param name The name used by this operation.
         * @return The result of the operation.
         */
        Entity createEntity(const std::string &name = {});

        /**
         * @brief Destroys entity and releases its resources.
         * @param entity The entity used by this operation.
         */
        void destroyEntity(Entity entity);

        /**
         * @brief Returns the all entities.
         * @return The result of the operation.
         */
        std::vector<std::shared_ptr<Entity>> getAllEntities();

        /**
         * @brief Renders 2 d.
         */
        void render2D();

        /**
         * @brief Returns the primary camera entity.
         * @return The result of the operation.
         */
        Entity getPrimaryCameraEntity();

        // flecs::query ist selbst leichtgewichtig genug, um pro Aufruf neu
        // gebaut zu werden (kein Cache noetig) - anders als bei entt::view
        // gibt es hier keine impliziten Lifetime-Fallstricke, weil die Query
        // nicht auf interne Registry-Iteratoren verweist.
        /**
         * @brief Returns the view.
         * @return The result of the operation.
         */
        template <typename... Components> auto view() {
            return m_world.query<Components...>();
        }

        /**
         * @brief Returns the view.
         * @return The result of the operation.
         */
        template <typename... Components> auto view() const {
            return m_world.query<const Components...>();
        }

        /**
         * @brief Returns the name.
         * @return The result of the operation.
         */
        std::string getName() const { return m_name; }

      private:
        /** @brief Stores the world. */
        flecs::world m_world;

        /** @brief Stores the name. */
        std::string m_name;

        friend class Entity;
    };

} // namespace axiom
