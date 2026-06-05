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
		Scene(const std::string& name = "Scene") : m_Name(name) {}

        Entity CreateEntity(const std::string& name = {});

        void DestroyEntity(Entity entity);

        std::vector<std::shared_ptr<Entity>> GetAllEntities();

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

		std::string GetName() const { return m_Name; }

    private:
        entt::registry m_Registry;

		std::string m_Name;

        friend class Entity;
    };

}
