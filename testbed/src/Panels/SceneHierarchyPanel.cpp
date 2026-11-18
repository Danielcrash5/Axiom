/**
 * @file testbed/src/Panels/SceneHierarchyPanel.cpp
 * @brief Implements the scene hierarchy panel module.
 */

#include "Panels/SceneHierarchyPanel.h"

namespace testbed {
	void SceneHierarchyPanel::onImGuiRender() {
		for (size_t i = 0; i < m_scenes.size(); i++) {
			ImGui::PushID(static_cast<int>(i));
			const auto& scene = m_scenes[i];
			if (ImGui::TreeNode(scene->getName().c_str())) {
				for (auto& entity : scene->getAllEntities()) {
					ImGui::PushID(static_cast<int>(entity->getId()));
					if (ImGui::Selectable(entity->getName().c_str(), m_selectedEntity && m_selectedEntity == entity)) {
						m_selectedEntity = entity;
						AXIOM_INFO("Selected Entity: {}", entity->getName());
						m_inspectorPanel->updateSelectedEntity(m_selectedEntity);
					}
					ImGui::PopID();
				}
				ImGui::TreePop();
			}
			ImGui::PopID();
		}
	}
}