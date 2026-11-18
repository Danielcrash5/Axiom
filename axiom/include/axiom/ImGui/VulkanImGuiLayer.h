/**
 * @file axiom/include/axiom/ImGui/VulkanImGuiLayer.h
 * @brief Declares the public interface for the vulkan ImGui layer.
 */

#pragma once

#include "axiom/ImGui/IImGuiLayer.h"

#include <vulkan/vulkan.h>

#include <cstdint>
#include <memory>
#include <vector>

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
    /**
     * @brief Provides the vulkan ImGui layer functionality.
     * @tparam VulkanImGuiLayer Template type used by this type.
     */
    class VulkanImGuiLayer final : public IImGuiLayer {
      public:
        /**
         * @brief Creates the requested resource.
         * @param window The window used by this operation.
         * @param renderer The renderer used by this operation.
         * @return The result of the operation.
         */
        [[nodiscard]] static std::unique_ptr<VulkanImGuiLayer>
        create(Window &window, renderer::Renderer &renderer);

        /**
         * @brief Releases resources owned by VulkanImGuiLayer.
         */
        ~VulkanImGuiLayer() override;

        /**
         * @brief Begins the operation.
         */
        void begin() override;
        /**
         * @brief Ends the operation.
         */
        void end() override;
        /**
         * @brief Processes event.
         * @param event The event used by this operation.
         * @return The result of the operation.
         */
        bool processEvent(const SDL_Event &event) override;
        /**
         * @brief Renders additional viewports.
         */
        void renderAdditionalViewports() override;

      private:
        /**
         * @brief Creates an instance of vulkan ImGui layer.
         */
        VulkanImGuiLayer() = default;

        /** @brief Stores the device. */
        VkDevice m_device = VK_NULL_HANDLE;
        // Muss so lange leben wie das Backend: imgui_impl_vulkan haelt den
        // Pointer aus PipelineRenderingCreateInfo::pColorAttachmentFormats.
        /** @brief Stores the color format. */
        VkFormat m_colorFormat = VK_FORMAT_UNDEFINED;
        /** @brief Stores the inter bold font data. */
        std::vector<uint8_t> m_interBoldFontData;
        /** @brief Stores the inter regular font data. */
        std::vector<uint8_t> m_interRegularFontData;
        /** @brief Stores the jet brains mono font data. */
        std::vector<uint8_t> m_jetBrainsMonoFontData;

        /** @brief Stores the context created. */
        bool m_contextCreated = false;
        /** @brief Stores the sdl backend ready. */
        bool m_sdlBackendReady = false;
        /** @brief Stores the vulkan backend ready. */
        bool m_vulkanBackendReady = false;
    };

} // namespace axiom
