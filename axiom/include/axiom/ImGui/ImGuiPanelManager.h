/**
 * @file axiom/include/axiom/ImGui/ImGuiPanelManager.h
 * @brief Declares the public interface for the ImGui panel manager.
 */

#pragma once

#include "IImGuiPanel.h"
#include <memory>
#include <vector>

namespace axiom {
    /**
     * @brief Provides the ImGui panel manager functionality.
     * @tparam ImGuiPanelManager Template type used by this type.
     */
    class ImGuiPanelManager {
      public:
        /**
         * @brief Creates an instance of ImGui panel manager.
         */
        ImGuiPanelManager() = default;
        /**
         * @brief Releases resources owned by ImGuiPanelManager.
         */
        ~ImGuiPanelManager() = default;

        /**
         * @brief Updates the object state.
         * @param deltaTime The delta time used by this operation.
         */
        void update(double deltaTime);

        /**
         * @brief Performs the ImGui render operation.
         */
        void imGuiRender();

        /**
         * @brief Adds panel.
         * @param panel The panel used by this operation.
         */
        void addPanel(const std::shared_ptr<IImGuiPanel> &panel);
        /**
         * @brief Removes panel.
         * @param name The name used by this operation.
         */
        void removePanel(const std::string &name);

      private:
        /** @brief Stores the panels. */
        std::vector<std::shared_ptr<IImGuiPanel>> m_panels;
    };
} // namespace axiom