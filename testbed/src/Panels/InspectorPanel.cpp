#include "Panels/InspectorPanel.h"
#include <any>

namespace testbed {
	void InspectorPanel::OnImGuiRender() {
		if (!m_SelectedEntity) {
			ImGui::Text("No entity selected.");
			return;
		}
		ImGui::Text("Selected Entity: %s", m_SelectedEntity->GetName().c_str());
		ImGui::Separator();
		// Display transformonents of the selected entity
		for (const auto& component : m_SelectedEntity->GetComponents()) {
			ImGui::Separator();
			auto Component = m_SelectedEntity->GetComponentByComponentInfo(component);
			ImGui::CollapsingHeader(component.GetName().c_str(), ImGuiTreeNodeFlags_DefaultOpen);
			if (Component.type() == typeid(axiom::TransformComponent)) {
				auto& transform = std::any_cast<axiom::TransformComponent&>(Component);
				ImGui::Text("Transform Component:");
				if (ImGui::BeginTable("TransformTable", 2, ImGuiTableFlags_SizingFixedFit)) {
					// Erste Spalte fix auf 100 Pixel, zweite Spalte dehnt sich aus
					ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed, 100.0f);
					ImGui::TableSetupColumn("Values", ImGuiTableColumnFlags_WidthStretch);

					// Zeilenhöhe stabil über die Public-API berechnen
					float lineHeight = ImGui::GetFontSize() + ImGui::GetStyle().FramePadding.y * 2.0f;
					ImVec2 buttonSize = {lineHeight + 3.0f, lineHeight};

					// ==========================================
					// 1. ZEILE: TRANSLATION
					// ==========================================
					ImGui::PushID("Translation");
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);
					ImGui::AlignTextToFramePadding();
					ImGui::Text("Translation");

					ImGui::TableSetColumnIndex(1);
					// Berechne die genaue Breite für jeden der 3 Slider (Verfügbare Breite minus Buttons, geteilt durch 3)
					float itemWidth = (ImGui::GetContentRegionAvail().x - (buttonSize.x * 3.0f)) / 3.0f;

					ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{0, 0});

					// X
					ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.8f, 0.1f, 0.15f, 1.0f});
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{0.9f, 0.2f, 0.2f, 1.0f});
					ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{0.8f, 0.1f, 0.15f, 1.0f});
					if (ImGui::Button("X", buttonSize)) transform.Translation.x = 0.0f;
					ImGui::PopStyleColor(3);
					ImGui::SameLine();
					ImGui::SetNextItemWidth(itemWidth);
					ImGui::DragFloat("##X", &transform.Translation.x, 0.1f, 0.0f, 0.0f, "%.2f");
					ImGui::SameLine();

					// Y
					ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.2f, 0.7f, 0.2f, 1.0f});
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{0.3f, 0.8f, 0.3f, 1.0f});
					ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{0.2f, 0.7f, 0.2f, 1.0f});
					if (ImGui::Button("Y", buttonSize)) transform.Translation.y = 0.0f;
					ImGui::PopStyleColor(3);
					ImGui::SameLine();
					ImGui::SetNextItemWidth(itemWidth);
					ImGui::DragFloat("##Y", &transform.Translation.y, 0.1f, 0.0f, 0.0f, "%.2f");
					ImGui::SameLine();

					// Z
					ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.1f, 0.25f, 0.8f, 1.0f});
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{0.2f, 0.35f, 0.9f, 1.0f});
					ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{0.1f, 0.25f, 0.8f, 1.0f});
					if (ImGui::Button("Z", buttonSize)) transform.Translation.z = 0.0f;
					ImGui::PopStyleColor(3);
					ImGui::SameLine();
					ImGui::SetNextItemWidth(itemWidth);
					ImGui::DragFloat("##Z", &transform.Translation.z, 0.1f, 0.0f, 0.0f, "%.2f");

					ImGui::PopStyleVar();
					ImGui::PopID();

					// ==========================================
					// 2. ZEILE: ROTATION
					// ==========================================
					ImGui::PushID("Rotation");
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);
					ImGui::AlignTextToFramePadding();
					ImGui::Text("Rotation");

					ImGui::TableSetColumnIndex(1);
					ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{0, 0});

					// X
					ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.8f, 0.1f, 0.15f, 1.0f});
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{0.9f, 0.2f, 0.2f, 1.0f});
					ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{0.8f, 0.1f, 0.15f, 1.0f});
					if (ImGui::Button("X", buttonSize)) transform.Rotation.x = 0.0f;
					ImGui::PopStyleColor(3);
					ImGui::SameLine();
					ImGui::SetNextItemWidth(itemWidth);
					ImGui::DragFloat("##X", &transform.Rotation.x, 0.1f, 0.0f, 0.0f, "%.2f");
					ImGui::SameLine();

					// Y
					ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.2f, 0.7f, 0.2f, 1.0f});
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{0.3f, 0.8f, 0.3f, 1.0f});
					ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{0.2f, 0.7f, 0.2f, 1.0f});
					if (ImGui::Button("Y", buttonSize)) transform.Rotation.y = 0.0f;
					ImGui::PopStyleColor(3);
					ImGui::SameLine();
					ImGui::SetNextItemWidth(itemWidth);
					ImGui::DragFloat("##Y", &transform.Rotation.y, 0.1f, 0.0f, 0.0f, "%.2f");
					ImGui::SameLine();

					// Z
					ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.1f, 0.25f, 0.8f, 1.0f});
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{0.2f, 0.35f, 0.9f, 1.0f});
					ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{0.1f, 0.25f, 0.8f, 1.0f});
					if (ImGui::Button("Z", buttonSize)) transform.Rotation.z = 0.0f;
					ImGui::PopStyleColor(3);
					ImGui::SameLine();
					ImGui::SetNextItemWidth(itemWidth);
					ImGui::DragFloat("##Z", &transform.Rotation.z, 0.1f, 0.0f, 0.0f, "%.2f");

					ImGui::PopStyleVar();
					ImGui::PopID();

					// ==========================================
					// 3. ZEILE: SCALE
					// ==========================================
					ImGui::PushID("Scale");
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);
					ImGui::AlignTextToFramePadding();
					ImGui::Text("Scale");

					ImGui::TableSetColumnIndex(1);
					ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{0, 0});

					// X
					ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.8f, 0.1f, 0.15f, 1.0f});
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{0.9f, 0.2f, 0.2f, 1.0f});
					ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{0.8f, 0.1f, 0.15f, 1.0f});
					if (ImGui::Button("X", buttonSize)) transform.Scale.x = 1.0f;
					ImGui::PopStyleColor(3);
					ImGui::SameLine();
					ImGui::SetNextItemWidth(itemWidth);
					ImGui::DragFloat("##X", &transform.Scale.x, 0.1f, 0.0f, 0.0f, "%.2f");
					ImGui::SameLine();

					// Y
					ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.2f, 0.7f, 0.2f, 1.0f});
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{0.3f, 0.8f, 0.3f, 1.0f});
					ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{0.2f, 0.7f, 0.2f, 1.0f});
					if (ImGui::Button("Y", buttonSize)) transform.Scale.y = 1.0f;
					ImGui::PopStyleColor(3);
					ImGui::SameLine();
					ImGui::SetNextItemWidth(itemWidth);
					ImGui::DragFloat("##Y", &transform.Scale.y, 0.1f, 0.0f, 0.0f, "%.2f");
					ImGui::SameLine();

					// Z
					ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.1f, 0.25f, 0.8f, 1.0f});
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{0.2f, 0.35f, 0.9f, 1.0f});
					ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{0.1f, 0.25f, 0.8f, 1.0f});
					if (ImGui::Button("Z", buttonSize)) transform.Scale.z = 1.0f;
					ImGui::PopStyleColor(3);
					ImGui::SameLine();
					ImGui::SetNextItemWidth(itemWidth);
					ImGui::DragFloat("##Z", &transform.Scale.z, 0.1f, 0.0f, 0.0f, "%.2f");

					ImGui::PopStyleVar();
					ImGui::PopID();

					ImGui::EndTable();
				}
			}
		}
	}
}