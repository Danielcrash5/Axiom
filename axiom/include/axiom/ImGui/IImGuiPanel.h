#pragma once

#include <string>
#include <imgui.h>

namespace axiom {
	class IImGuiPanel {
	public:
		IImGuiPanel(const std::string& name) : m_Name(name) {}

		virtual ~IImGuiPanel() = default;

		const std::string& GetName() const { return m_Name; }

		inline void ImGuiRender() {
			ImGui::PushID(m_Name.c_str());
			ImGui::Begin(m_Name.c_str());
			OnImGuiRender();
			ImGui::End();
			ImGui::PopID();
		}

		virtual void OnUpdate(double deltaTime) {}

		virtual void OnImGuiRender() = 0;
	private:
		std::string m_Name;
	};
}