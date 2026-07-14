#include "axiom/ImGui/ImGuiPanelManager.h"

#include <algorithm>

namespace axiom {
void ImGuiPanelManager::Update(double deltaTime) {
    for (const auto &panel : m_Panels) {
        panel->OnUpdate(deltaTime);
    }
}

void ImGuiPanelManager::ImGuiRender() {
    for (const auto &panel : m_Panels) {
        panel->ImGuiRender();
    }
}

void ImGuiPanelManager::AddPanel(const std::shared_ptr<IImGuiPanel> &panel) {
    m_Panels.push_back(panel);
}

void ImGuiPanelManager::RemovePanel(const std::string &name) {
    m_Panels.erase(
        std::remove_if(m_Panels.begin(), m_Panels.end(),
                       [&name](const std::shared_ptr<IImGuiPanel> &panel) {
                           return panel->GetName() == name;
                       }),
        m_Panels.end());
}
} // namespace axiom
