#include "axiom/ImGui/IImGuiLayer.h"
#include "axiom/core/Logger.h"
#include "axiom/platform/Window.h"
#include <memory>

namespace axiom {
std::unique_ptr<IImGuiLayer>
IImGuiLayer::Create(std::unique_ptr<Window> &window) {
    return nullptr;
}
} // namespace axiom