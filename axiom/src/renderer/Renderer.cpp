#include "axiom/renderer/Renderer.h"

#include "axiom/platform/Window.h"

#include <stdexcept>

namespace axiom {
    void Renderer::Initialize(
        std::shared_ptr<Window> window,
        RendererAPI api) {
        m_API = api;

        switch (m_API) {
        case RendererAPI::Vulkan:
        {
            InitializeVulkan(window);
            break;
        }

        case RendererAPI::OpenGL:
        {
            InitializeOpenGL(window);
            break;
        }

        default:
        {
            throw std::runtime_error(
                "Unsupported renderer API.");
        }
        }
    }

    void Renderer::Shutdown() {
        switch (m_API) {
        case RendererAPI::Vulkan:
        {
            m_Swapchain.Destroy();

            m_Device.Destroy();

            m_Context.Shutdown();

            break;
        }

        default:
            break;
        }
    }

    void Renderer::BeginFrame() {
        switch (m_API) {
        case RendererAPI::Vulkan:
        {
            // später:
            // Acquire Swapchain Image
            // Begin Command Buffer

            break;
        }

        default:
            break;
        }
    }

    void Renderer::EndFrame() {
        switch (m_API) {
        case RendererAPI::Vulkan:
        {
            // später:
            // Submit
            // Present

            break;
        }

        default:
            break;
        }
    }

    void Renderer::InitializeVulkan(
        std::shared_ptr<Window> window) {
        m_Context.Initialize(
            window->GetNativeHandle());

        m_PhysicalDevice.Pick(
            m_Context.GetInstance(),
            m_Context.GetSurface());

        m_Device.Create(
            m_PhysicalDevice);

        m_Swapchain.Create(
            m_Context,
            m_PhysicalDevice,
            m_Device,
            static_cast<uint32_t>(window->GetWidth()),
            static_cast<uint32_t>(window->GetHeight()));
    }

    void Renderer::InitializeOpenGL(
        std::shared_ptr<Window> window) {
        throw std::runtime_error(
            "OpenGL backend not implemented.");
    }
}