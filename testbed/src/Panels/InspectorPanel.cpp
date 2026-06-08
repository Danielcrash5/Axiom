#include "Panels/InspectorPanel.h"

#include <axiom/Axiom.h>
#include <imgui_internal.h>
#include <glm/gtc/type_ptr.hpp>

namespace testbed {
	static void DrawVec3Control(const std::string& label, glm::vec3& values, float resetValue = 0.0f, float columnWidth = 100.0f) {
		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[0];

		ImGui::PushID(label.c_str());

		ImGui::Columns(2);
		ImGui::SetColumnWidth(0, columnWidth);
		ImGui::Text(label.c_str());
		ImGui::NextColumn();

		ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{0, 0});

		float lineHeight = ImGui::GetFontSize() + ImGui::GetStyle().FramePadding.y * 2.0f;
		ImVec2 buttonSize = {lineHeight + 3.0f, lineHeight};

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.8f, 0.1f, 0.15f, 1.0f});
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{0.9f, 0.2f, 0.2f, 1.0f});
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{0.8f, 0.1f, 0.15f, 1.0f});
		ImGui::PushFont(boldFont);
		if (ImGui::Button("X", buttonSize))
			values.x = resetValue;
		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::DragFloat("##X", &values.x, 0.1f, 0.0f, 0.0f, "%.2f");
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.2f, 0.7f, 0.2f, 1.0f});
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{0.3f, 0.8f, 0.3f, 1.0f});
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{0.2f, 0.7f, 0.2f, 1.0f});
		ImGui::PushFont(boldFont);
		if (ImGui::Button("Y", buttonSize))
			values.y = resetValue;
		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::DragFloat("##Y", &values.y, 0.1f, 0.0f, 0.0f, "%.2f");
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.1f, 0.25f, 0.8f, 1.0f});
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{0.2f, 0.35f, 0.9f, 1.0f});
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{0.1f, 0.25f, 0.8f, 1.0f});
		ImGui::PushFont(boldFont);
		if (ImGui::Button("Z", buttonSize))
			values.z = resetValue;
		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::DragFloat("##Z", &values.z, 0.1f, 0.0f, 0.0f, "%.2f");
		ImGui::PopItemWidth();

		ImGui::PopStyleVar();

		ImGui::Columns(1);

		ImGui::PopID();
	}

	void InspectorPanel::OnImGuiRender() {
		if (!m_SelectedEntity)
			return;
		auto& entity = *m_SelectedEntity;
		DrawComponent<axiom::TagComponent>("Tag", entity.GetComponent<axiom::TagComponent>(), [](auto& tag) {
			char buffer[256];
			std::strncpy(buffer, tag.Tag.c_str(), sizeof(buffer));
			if (ImGui::InputText("##Tag", buffer, sizeof(buffer))) {
				tag.Tag = std::string(buffer);
			}
									});
		DrawComponent<axiom::TransformComponent>("Transform", entity.GetComponent<axiom::TransformComponent>(), [](auto& transform) {
			DrawVec3Control("Translation", transform.Translation);
			DrawVec3Control("Rotation", transform.Rotation);
			DrawVec3Control("Scale", transform.Scale, 1.0f);
					  });
		if (entity.HasComponent<axiom::SpriteRendererComponent>()) {
			DrawComponent<axiom::SpriteRendererComponent>("Sprite Renderer", entity.GetComponent<axiom::SpriteRendererComponent>(), [](auto& spriteRenderer) {
				ImGui::ColorEdit4("Color", glm::value_ptr(spriteRenderer.Color));
				ImGui::DragFloat("Tiling Factor", &spriteRenderer.TilingFactor, 0.1f, 0.0f, 100.0f);
						  });
		}
		if (entity.HasComponent<axiom::CameraComponent>()) {
			DrawComponent<axiom::CameraComponent>("Camera", entity.GetComponent<axiom::CameraComponent>(), [](auto& camera) {
				ImGui::Checkbox("Primary", &camera.Primary);
				ImGui::DragFloat("Orthographic Size", &camera.OrthographicSize, 0.1f, 0.0f, 1000.0f);
				ImGui::DragFloat("Near Clip", &camera.NearClip, 0.1f, -1000.0f, 1000.0f);
				ImGui::DragFloat("Far Clip", &camera.FarClip, 0.1f, -1000.0f, 1000.0f);
												  });
		}
		if (entity.HasComponent<axiom::CircleRendererComponent>()) {
			DrawComponent<axiom::CircleRendererComponent>("Circle Renderer", entity.GetComponent<axiom::CircleRendererComponent>(), [](auto& circleRenderer) {
				ImGui::ColorEdit4("Color", glm::value_ptr(circleRenderer.Color));
				ImGui::DragFloat("Thickness", &circleRenderer.Thickness, 0.1f, 0.0f, 100.0f);
														  });
		}
		
	}
}