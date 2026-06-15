#pragma once

#include <Volk/volk.h>

struct SDL_Window;

namespace axiom {
    class VulkanContext {
    public:
        VulkanContext() = default;
        ~VulkanContext() = default;

        void Initialize(SDL_Window* window);
        void Shutdown();

        VkInstance GetInstance() const {
            return m_Instance;
        }

        VkSurfaceKHR GetSurface() const {
            return m_Surface;
        }

    private:
        void CreateInstance();
        void CreateSurface(SDL_Window* window);

    private:
        VkInstance m_Instance = VK_NULL_HANDLE;
        VkSurfaceKHR m_Surface = VK_NULL_HANDLE;
    };
}