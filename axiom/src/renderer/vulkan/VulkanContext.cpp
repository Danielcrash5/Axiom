#include "axiom/renderer/vulkan/VulkanContext.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

#include <vector>
#include <stdexcept>

namespace {
#ifdef _DEBUG
    constexpr bool EnableValidationLayers = true;
#else
    constexpr bool EnableValidationLayers = false;
#endif

    const std::vector<const char*> ValidationLayers =
    {
        "VK_LAYER_KHRONOS_validation"
    };
}

namespace axiom {
    void VulkanContext::Initialize(SDL_Window* window) {
        if (volkInitialize() != VK_SUCCESS) {
            throw std::runtime_error(
                "Failed to initialize Volk.");
        }

        CreateInstance();

        volkLoadInstance(m_Instance);

        CreateSurface(window);
    }

    void VulkanContext::Shutdown() {
        if (m_Surface != VK_NULL_HANDLE) {
            vkDestroySurfaceKHR(
                m_Instance,
                m_Surface,
                nullptr);

            m_Surface = VK_NULL_HANDLE;
        }

        if (m_Instance != VK_NULL_HANDLE) {
            vkDestroyInstance(
                m_Instance,
                nullptr);

            m_Instance = VK_NULL_HANDLE;
        }
    }

    void VulkanContext::CreateInstance() {
        uint32_t extensionCount = 0;

        const char* const* sdlExtensions =
            SDL_Vulkan_GetInstanceExtensions(
                &extensionCount);

        if (!sdlExtensions) {
            throw std::runtime_error(
                "Failed to get SDL Vulkan extensions.");
        }

        std::vector<const char*> extensions(
            sdlExtensions,
            sdlExtensions + extensionCount);

        if constexpr (EnableValidationLayers) {
            extensions.push_back(
                VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        VkApplicationInfo appInfo {};
        appInfo.sType =
            VK_STRUCTURE_TYPE_APPLICATION_INFO;

        appInfo.pApplicationName =
            "AxiomEngine";

        appInfo.applicationVersion =
            VK_MAKE_VERSION(1, 0, 0);

        appInfo.pEngineName =
            "AxiomEngine";

        appInfo.engineVersion =
            VK_MAKE_VERSION(1, 0, 0);

        appInfo.apiVersion =
            VK_API_VERSION_1_4;

        VkInstanceCreateInfo createInfo {};
        createInfo.sType =
            VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;

        createInfo.pApplicationInfo =
            &appInfo;

        createInfo.enabledExtensionCount =
            static_cast<uint32_t>(
                extensions.size());

        createInfo.ppEnabledExtensionNames =
            extensions.data();

        if constexpr (EnableValidationLayers) {
            createInfo.enabledLayerCount =
                static_cast<uint32_t>(
                    ValidationLayers.size());

            createInfo.ppEnabledLayerNames =
                ValidationLayers.data();
        }
        else {
            createInfo.enabledLayerCount = 0;
        }

        VkResult result =
            vkCreateInstance(
                &createInfo,
                nullptr,
                &m_Instance);

        if (result != VK_SUCCESS) {
            throw std::runtime_error(
                "Failed to create Vulkan instance.");
        }
    }

    void VulkanContext::CreateSurface(
        SDL_Window* window) {
        if (!SDL_Vulkan_CreateSurface(
            window,
            m_Instance,
            nullptr,
            &m_Surface)) {
            throw std::runtime_error(
                "Failed to create Vulkan surface.");
        }
    }
}