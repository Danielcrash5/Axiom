#pragma once
#include <flecs.h>
#include <memory>
#include <optional>
#include <string>
#include <utility>

#include "Components.h"

namespace axiom {

    class Entity;

    class Scene {
      public:
        Scene(const std::string &name = "Scene") : m_Name(name) {}

        Entity CreateEntity(const std::string &name = {});

        void DestroyEntity(Entity entity);

        std::vector<std::shared_ptr<Entity>> GetAllEntities();

        void Render2D();

        Entity GetPrimaryCameraEntity();

        // flecs::query ist selbst leichtgewichtig genug, um pro Aufruf neu
        // gebaut zu werden (kein Cache noetig) - anders als bei entt::view
        // gibt es hier keine impliziten Lifetime-Fallstricke, weil die Query
        // nicht auf interne Registry-Iteratoren verweist.
        template <typename... Components> auto View() {
            return m_World.query<Components...>();
        }

        template <typename... Components> auto View() const {
            return m_World.query<const Components...>();
        }

        std::string GetName() const { return m_Name; }

      private:
        flecs::world m_World;

        std::string m_Name;

        friend class Entity;
    };

} // namespace axiom
