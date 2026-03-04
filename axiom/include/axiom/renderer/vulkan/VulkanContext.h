#pragma once
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <set>

class VulkanContext {
public:
    void Init(GLFWwindow* window);
    void Cleanup();

    VkInstance GetInstance() const {
        return m_Instance;
    }
    VkPhysicalDevice GetPhysicalDevice() const {
        return m_PhysicalDevice;
    }
    VkDevice GetDevice() const {
        return m_Device;
    }
    VkSurfaceKHR GetSurface() const {
        return m_Surface;
    }

    uint32_t GetGraphicsQueueFamilyIndex() const {
        return m_GraphicsQueueIndex;
    }
    uint32_t GetPresentQueueFamilyIndex() const {
        return m_PresentQueueIndex;
    }

private:
    void CreateInstance();
    void CreateSurface(GLFWwindow* window);
    void PickPhysicalDevice();
    void FindQueueFamilies();
    void CreateLogicalDevice();

private:
    VkInstance m_Instance = VK_NULL_HANDLE;
    VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
    VkDevice m_Device = VK_NULL_HANDLE;
    VkSurfaceKHR m_Surface = VK_NULL_HANDLE;

    uint32_t m_GraphicsQueueIndex = UINT32_MAX;
    uint32_t m_PresentQueueIndex = UINT32_MAX;
};