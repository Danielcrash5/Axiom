#pragma once

#include "RendererAPI.h"

#include "axiom/renderer/vulkan/VulkanContext.h"
#include "axiom/renderer/vulkan/VulkanPhysicalDevice.h"
#include "axiom/renderer/vulkan/VulkanDevice.h"
#include "axiom/renderer/vulkan/VulkanSwapchain.h"

#include <memory>

namespace axiom {
    class Window;

    class Renderer {
    public:
        Renderer() = default;
        ~Renderer() = default;

        void Initialize(
            std::shared_ptr<Window> window,
            RendererAPI api);

        void Shutdown();

        void BeginFrame();
        void EndFrame();

        RendererAPI GetAPI() const {
            return m_API;
        }

    private:
        void InitializeVulkan(std::shared_ptr<Window> window);
        void InitializeOpenGL(std::shared_ptr<Window> window);

    private:
        RendererAPI m_API = RendererAPI::Vulkan;

        VulkanContext m_Context;
        VulkanPhysicalDevice m_PhysicalDevice;
        VulkanDevice m_Device;
        VulkanSwapchain m_Swapchain;
    };
}