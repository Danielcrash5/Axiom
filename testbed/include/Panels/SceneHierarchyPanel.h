#pragma once
#include <axiom/axiom.h>

namespace testbed {
	class SceneHierarchyPanel : public axiom::IImGuiPanel {
	public:
		explicit SceneHierarchyPanel(std::vector<std::unique_ptr<axiom::Scene>>& m_Scenes)
			: IImGuiPanel("Scene Hierarchy")
			, m_Scenes(m_Scenes) {
		}
		void OnImGuiRender() override;
	private:
		std::vector<std::unique_ptr<axiom::Scene>>& m_Scenes;
	};
}