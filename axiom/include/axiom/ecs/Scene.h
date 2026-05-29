#pragma once
#include <entt/entt.hpp>
#include <memory>
#include <optional>
#include <string>
#include <utility>

#include "Components.h"

namespace axiom {

    class Entity;

    class Scene {
    public:
        Entity CreateEntity(const std::string& name = {});

        void DestroyEntity(Entity entity);
        void Render2D();

        Entity GetPrimaryCameraEntity();

        template<typename... Components>
        auto View() {
            return m_Registry.view<Components...>();
        }

        template<typename... Components>
        auto View() const {
            return m_Registry.view<const Components...>();
        }

    private:
        entt::registry m_Registry;

        friend class Entity;
    };

}
