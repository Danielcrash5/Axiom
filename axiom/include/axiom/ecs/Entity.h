#pragma once
#include <utility>
#include <any>

#include <entt/entt.hpp>
#include "Scene.h"

namespace axiom {


	class Entity {
	public:
		Entity() = default;
		Entity(entt::entity handle, Scene* scene)
			: m_Entity(handle), m_Scene(scene) {
		}

		entt::entity GetID() const {
			return m_Entity;
		}

		bool operator ==(const Entity& other) const {
			return m_Entity == other.m_Entity && m_Scene == other.m_Scene;
		}

		std::string GetName() const {
			return m_Scene->m_Registry.get<TagComponent>(m_Entity).Tag;
		}

		void SetName(const std::string& name) {
			auto& tag = m_Scene->m_Registry.get<TagComponent>(m_Entity);
			tag.Tag = name;
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

		struct ComponentInfo {
			entt::id_type id;
			
			const entt::sparse_set* storage;

			std::string GetName() const {
				if (storage) {
					std::string name{storage->info().name()};
					return name;
				}
				return "Unknown Component";
			}
		};

		std::vector<ComponentInfo> GetComponents() const {
			std::vector<ComponentInfo> result;

			
			for (auto&& [id, storage] : m_Scene->m_Registry.storage()) {
				if (storage.contains(m_Entity)) {
					
					result.push_back({id, &storage});
				}
			}

			return result;
		}

		std::any GetComponentById(entt::id_type id) const {
			auto* storage = m_Scene->m_Registry.storage(id);
			if (storage && storage->contains(m_Entity)) {
				// Wir geben den rohen Pointer verpackt in std::any zurück
				return std::any(storage->value(m_Entity));
			}
			return std::any(); // Gibt ein leeres std::any zurück
		}

		std::any GetComponentByComponentInfo(const ComponentInfo& info) const {
			if (info.storage && info.storage->contains(m_Entity)) {
				return std::any(info.storage->value(m_Entity));
			}
			return std::any();
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
