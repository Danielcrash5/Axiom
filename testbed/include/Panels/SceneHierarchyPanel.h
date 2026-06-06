#pragma once
#include <axiom/axiom.h>
#include "InspectorPanel.h"
#include <memory>

namespace testbed {
	class SceneHierarchyPanel : public axiom::IImGuiPanel {
	public:
		SceneHierarchyPanel(std::vector<std::unique_ptr<axiom::Scene>>& m_Scenes, std::shared_ptr<InspectorPanel> inspectorPanel)
			: IImGuiPanel("Scene Hierarchy")
			, m_Scenes(m_Scenes)
			, m_InspectorPanel(inspectorPanel) {
		}
		void OnImGuiRender() override;
	private:
		std::vector<std::unique_ptr<axiom::Scene>>& m_Scenes;
		std::shared_ptr<axiom::Entity> m_SelectedEntity;
		std::shared_ptr<InspectorPanel> m_InspectorPanel;
	};
}