#pragma once
#include <entt/entt.hpp>
#include "Scene.h"

namespace axiom {


    class Entity {
    public:
        Entity() = default;
        Entity(entt::entity handle, Scene* scene)
            : m_Entity(handle), m_Scene(scene) {
        }

        template<typename T, typename... Args>
        T& AddComponent(Args&&... args) {
            return m_Scene->m_Registry.emplace<T>(
                m_Entity, std::forward<Args>(args)...);
        }

        template<typename T>
        T& GetComponent() {
            return m_Scene->m_Registry.get<T>(m_Entity);
        }

        template<typename T>
        bool HasComponent() const {
            return m_Scene->m_Registry.all_of<T>(m_Entity);
        }

        operator bool() const {
            return m_Entity != entt::null;
        }

    private:
        entt::entity m_Entity { entt::null };
        Scene* m_Scene = nullptr;

        friend class Scene;
    };
}