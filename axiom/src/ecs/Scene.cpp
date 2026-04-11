#include "axiom/ecs/Scene.h"
#include "axiom/ecs/Entity.h"

namespace axiom {
	Entity Scene::CreateEntity() {
		return Entity(m_Registry.create(), this);
	}

	void Scene::DestroyEntity(Entity entity) {
		m_Registry.destroy(entity.m_Entity);
	}
}