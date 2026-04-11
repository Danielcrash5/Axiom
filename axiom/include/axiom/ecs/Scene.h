#pragma once
#include <entt/entt.hpp>

namespace axiom {

    class Entity;

    class Scene {
    public:
        Entity CreateEntity();

        void DestroyEntity(Entity entity);

        template<typename... Components>
        auto View() {
            return m_Registry.view<Components...>();
        }

    private:
        entt::registry m_Registry;

        friend class Entity;
    };

}