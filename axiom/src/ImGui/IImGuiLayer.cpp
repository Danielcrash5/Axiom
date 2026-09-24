#include "axiom/ImGui/IImGuiLayer.h"
#include "axiom/ImGui/VulkanImGuiLayer.h"

namespace axiom {
    std::unique_ptr<IImGuiLayer> IImGuiLayer::Create(Window &window,
                                                     renderer::Renderer &renderer) {
        return VulkanImGuiLayer::Create(window, renderer);
    }
} // namespace axiom
