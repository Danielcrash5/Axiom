#pragma once
#include <axiom/axiom.h>
#include <memory>
#include <string>
#include <functional>

namespace testbed {
	template<typename T>
	void DrawComponent(const std::string& name, T& component, std::function<void(T&)> drawFunction) {
		ImGui::PushID(name.c_str());
		ImGui::Text("%s", name.c_str());
		ImGui::Separator();
		drawFunction(component);
		ImGui::PopID();
	}

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