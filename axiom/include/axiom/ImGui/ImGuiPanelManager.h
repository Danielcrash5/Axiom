#pragma once

#include "IImGuiPanel.h"
#include <memory>
#include <vector>

namespace axiom {
class ImGuiPanelManager {
  public:
    ImGuiPanelManager() = default;
    ~ImGuiPanelManager() = default;

    void Update(double deltaTime);

    void ImGuiRender();

    void AddPanel(const std::shared_ptr<IImGuiPanel> &panel);
    void RemovePanel(const std::string &name);

  private:
    std::vector<std::shared_ptr<IImGuiPanel>> m_Panels;
};
} // namespace axiom