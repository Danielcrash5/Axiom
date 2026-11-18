/**
 * @file axiom/src/ImGui/IImGuiLayer.cpp
 * @brief Implements the i ImGui layer module.
 */

#include "axiom/ImGui/IImGuiLayer.h"
#include "axiom/ImGui/VulkanImGuiLayer.h"

namespace axiom {
    std::unique_ptr<IImGuiLayer> IImGuiLayer::create(Window &window,
                                                     renderer::Renderer &renderer) {
        return VulkanImGuiLayer::create(window, renderer);
    }
} // namespace axiom
