#include "Panels/SceneHierarchyPanel.h"

namespace testbed {
	void SceneHierarchyPanel::OnImGuiRender() {
		for (size_t i = 0; i < m_Scenes.size(); i++) {
			ImGui::PushID(static_cast<int>(i));
			const auto& scene = m_Scenes[i];
			if (ImGui::TreeNode(scene->GetName().c_str())) {
				for (auto& entity : scene->GetAllEntities()) {
					ImGui::Text("%s", entity->GetName().c_str());
				}
				ImGui::TreePop();
			}
			ImGui::PopID();
		}
	}
}