/**
 * @file axiom/src/ImGui/ImGuiPanelManager.cpp
 * @brief Implements the ImGui panel manager module.
 */

#include "axiom/ImGui/ImGuiPanelManager.h"

#include <algorithm>

namespace axiom {
    void ImGuiPanelManager::update(double deltaTime) {
        for (const auto &panel : m_panels) {
            panel->onUpdate(deltaTime);
        }
    }

    void ImGuiPanelManager::imGuiRender() {
        for (const auto &panel : m_panels) {
            panel->imGuiRender();
        }
    }

    void
    ImGuiPanelManager::addPanel(const std::shared_ptr<IImGuiPanel> &panel) {
        m_panels.push_back(panel);
    }

    void ImGuiPanelManager::removePanel(const std::string &name) {
        m_panels.erase(
            std::remove_if(m_panels.begin(), m_panels.end(),
                           [&name](const std::shared_ptr<IImGuiPanel> &panel) {
                               return panel->getName() == name;
                           }),
            m_panels.end());
    }
} // namespace axiom
