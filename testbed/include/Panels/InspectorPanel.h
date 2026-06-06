#pragma once
#include <axiom/axiom.h>
#include <memory>

namespace testbed {
	class InspectorPanel : public axiom::IImGuiPanel {
	public:
		InspectorPanel()
			: IImGuiPanel("Inspector") {
		}
		void OnImGuiRender() override;
		void UpdateSelectedEntity(const std::shared_ptr<axiom::Entity>& entity) {
			m_SelectedEntity = entity;
		}
	private:
		std::shared_ptr<axiom::Entity> m_SelectedEntity;
	};
}