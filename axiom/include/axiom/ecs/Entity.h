#pragma once
#include <utility>

#include <entt/entt.hpp>
#include "Scene.h"

namespace axiom {


	class Entity {
	public:
		Entity() = default;
		Entity(entt::entity handle, Scene* scene)
			: m_Entity(handle), m_Scene(scene) {
		}

		void Destroy() {
			m_Scene->m_Registry.destroy(m_Entity);
		}

		template<typename T, typename... Args>
		T& AddComponent(Args&&... args) {
			return m_Scene->m_Registry.emplace<T>(
				m_Entity, std::forward<Args>(args)...);
		}

		template<typename T, typename... Args>
		T& AddOrReplaceComponent(Args&&... args) {
			return m_Scene->m_Registry.emplace_or_replace<T>(
				m_Entity, std::forward<Args>(args)...);
		}

		template<typename T, typename... Args>
		T& ReplaceComponent(Args&&... args) {
			return m_Scene->m_Registry.replace<T>(
				m_Entity, std::forward<Args>(args)...);
		}

		template<typename T>
		void RemoveComponent() {
			m_Scene->m_Registry.remove<T>(m_Entity);
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

		operator entt::entity() const {
			return m_Entity;
		}

	private:
		entt::entity m_Entity{entt::null};
		Scene* m_Scene = nullptr;

		friend class Scene;
	};
}
