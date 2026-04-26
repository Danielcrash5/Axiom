#include "axiom/ecs/Scene.h"
#include "axiom/ecs/Entity.h"
#include "axiom/renderer/Renderer2D.h"

namespace axiom {
	Entity Scene::CreateEntity(const std::string& name) {
		Entity entity(m_Registry.create(), this);
		entity.AddComponent<IDComponent>();
		entity.AddComponent<TagComponent>(name.empty() ? "Entity" : name);
		entity.AddComponent<TransformComponent>();
		return entity;
	}

	void Scene::DestroyEntity(Entity entity) {
		m_Registry.destroy(entity.m_Entity);
	}

	void Scene::Render2D() {
		auto spriteView = m_Registry.view<TransformComponent, SpriteRendererComponent>();
		for (auto entity : spriteView) {
			auto& transform = spriteView.get<TransformComponent>(entity);
			auto& spriteRenderer = spriteView.get<SpriteRendererComponent>(entity);

			if (spriteRenderer.HasTexture()) {
				Renderer2D::DrawSprite(transform.GetTransform(), spriteRenderer.SpriteData, spriteRenderer.Color);
				continue;
			}

			Renderer2D::DrawQuad(transform.GetTransform(), spriteRenderer.Color);
		}

		auto circleView = m_Registry.view<TransformComponent, CircleRendererComponent>();
		for (auto entity : circleView) {
			auto& transform = circleView.get<TransformComponent>(entity);
			auto& circleRenderer = circleView.get<CircleRendererComponent>(entity);
			Renderer2D::DrawCircle(transform.GetTransform(), circleRenderer.Thickness, circleRenderer.Color);
		}
	}
}
