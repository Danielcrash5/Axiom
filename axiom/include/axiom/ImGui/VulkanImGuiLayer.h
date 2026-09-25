#pragma once

#include "axiom/ImGui/IImGuiLayer.h"

#include <vulkan/vulkan.h>

#include <memory>

namespace axiom {

    namespace renderer {
        class Renderer;
    }

    // ImGui-Layer auf Basis der offiziellen Backends imgui_impl_sdl3 +
    // imgui_impl_vulkan (Dynamic Rendering). Registriert beim Erzeugen einen
    // eigenen ImGuiPass im RenderGraph des Renderers (schreibt aufs
    // Swapchain-Image des Hauptfensters und cleart es per loadOp).
    //
    // Frame-Ablauf (siehe Application::MainLoop):
    //   Begin() -> Panels zeichnen -> End()   // ImGui::Render(), nur CPU
    //   Renderer::renderFrame()               // ImGuiPass::execute() zeichnet
    //
    // Lebensdauer: MUSS vor dem Renderer/Device zerstoert werden.
    class VulkanImGuiLayer final : public IImGuiLayer {
      public:
        [[nodiscard]] static std::unique_ptr<VulkanImGuiLayer>
        Create(Window &window, renderer::Renderer &renderer);

        ~VulkanImGuiLayer() override;

        void Begin() override;
        void End() override;
        bool ProcessEvent(const SDL_Event &event) override;
        void RenderAdditionalViewports() override;

      private:
        VulkanImGuiLayer() = default;

        VkDevice m_Device = VK_NULL_HANDLE;
        // Muss so lange leben wie das Backend: imgui_impl_vulkan haelt den
        // Pointer aus PipelineRenderingCreateInfo::pColorAttachmentFormats.
        VkFormat m_ColorFormat = VK_FORMAT_UNDEFINED;

        bool m_ContextCreated = false;
        bool m_SDLBackendReady = false;
        bool m_VulkanBackendReady = false;
    };

} // namespace axiom
