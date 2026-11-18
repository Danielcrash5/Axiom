/**
 * @file testbed/include/Panels/SceneHierarchyPanel.h
 * @brief Declares the public interface for the scene hierarchy panel.
 */

#pragma once
#include <axiom/Axiom.h>
#include "InspectorPanel.h"
#include <memory>

namespace testbed {
	/**
	 * @brief Provides the scene hierarchy panel functionality.
	 * @tparam SceneHierarchyPanel Template type used by this type.
	 */
	class SceneHierarchyPanel : public axiom::IImGuiPanel {
	public:
		/**
		 * @brief Creates an instance of scene hierarchy panel.
		 * @param mScenes The m scenes used by this operation.
		 * @param inspectorPanel The inspector panel used by this operation.
		 */
		SceneHierarchyPanel(std::vector<std::unique_ptr<axiom::Scene>>& mScenes, std::shared_ptr<InspectorPanel> inspectorPanel)
			: IImGuiPanel("Scene Hierarchy")
			, m_scenes(mScenes)
			, m_inspectorPanel(inspectorPanel) {
		}
		/**
		 * @brief Handles the ImGui render callback.
		 */
		void onImGuiRender() override;
	private:
		/** @brief Stores the scenes. */
		std::vector<std::unique_ptr<axiom::Scene>>& m_scenes;
		/** @brief Stores the selected entity. */
		std::shared_ptr<axiom::Entity> m_selectedEntity;
		/** @brief Stores the inspector panel. */
		std::shared_ptr<InspectorPanel> m_inspectorPanel;
	};
}