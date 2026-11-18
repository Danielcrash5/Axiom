/**
 * @file testbed/include/Panels/InspectorPanel.h
 * @brief Declares the public interface for the inspector panel.
 */

#pragma once
#include <axiom/Axiom.h>
#include <memory>
#include <string>
#include <functional>

namespace testbed {
	/**
	 * @brief Draws component.
	 * @tparam T Template type used by this operation.
	 * @param name The name used by this operation.
	 * @param component The component used by this operation.
	 * @param drawFunction The draw function used by this operation.
	 */
	template<typename T>
	void drawComponent(const std::string& name, T& component, std::function<void(T&)> drawFunction) {
		ImGui::PushID(name.c_str());
		ImGui::Text("%s", name.c_str());
		ImGui::Separator();
		drawFunction(component);
		ImGui::PopID();
	}

	/**
	 * @brief Provides the inspector panel functionality.
	 * @tparam InspectorPanel Template type used by this type.
	 */
	class InspectorPanel : public axiom::IImGuiPanel {
	public:
		/**
		 * @brief Creates an instance of inspector panel.
		 */
		InspectorPanel()
			: IImGuiPanel("Inspector") {
		}

		/**
		 * @brief Handles the ImGui render callback.
		 */
		void onImGuiRender() override;

		/**
		 * @brief Updates selected entity.
		 * @param entity The entity used by this operation.
		 */
		void updateSelectedEntity(const std::shared_ptr<axiom::Entity>& entity) {
			m_selectedEntity = entity;
		}

	private:
		/** @brief Stores the selected entity. */
		std::shared_ptr<axiom::Entity> m_selectedEntity;
	};
}