#include "Panels/SceneHierarchyPanel.h"

namespace testbed {
	void SceneHierarchyPanel::OnImGuiRender() {
		for (size_t i = 0; i < m_Scenes.size(); i++) {
			ImGui::PushID(static_cast<int>(i));
			const auto& scene = m_Scenes[i];
			if (ImGui::TreeNode(scene->GetName().c_str())) {
				for (auto& entity : scene->GetAllEntities()) {
					ImGui::PushID(static_cast<int>(entity->GetID()));
					if (ImGui::Selectable(entity->GetName().c_str(), m_SelectedEntity && m_SelectedEntity == entity)) {
						m_SelectedEntity = entity;
						AXIOM_INFO("Selected Entity: {}", entity->GetName());
						m_InspectorPanel->UpdateSelectedEntity(m_SelectedEntity);
					}
					ImGui::PopID();
				}
				ImGui::TreePop();
			}
			ImGui::PopID();
		}
	}
}