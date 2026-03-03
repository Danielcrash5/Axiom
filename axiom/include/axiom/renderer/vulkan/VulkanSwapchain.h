#pragma once
#include "VulkanContext.h"
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <stdexcept>

class VulkanSwapchain {
public:
    VulkanSwapchain(VulkanContext* context, GLFWwindow* window);
    ~VulkanSwapchain();

    void Init();
    void Cleanup();

    VkSwapchainKHR GetSwapchain() const { return m_Swapchain; }
    const std::vector<VkImage>& GetImages() const { return m_Images; }
    VkFormat GetImageFormat() const { return m_ImageFormat; }
    VkExtent2D GetExtent() const { return m_Extent; }

private:
    VulkanContext* m_Context = nullptr;
    GLFWwindow* m_Window = nullptr;

    VkSwapchainKHR m_Swapchain = VK_NULL_HANDLE;
    std::vector<VkImage> m_Images;
    VkFormat m_ImageFormat = VK_FORMAT_UNDEFINED;
    VkExtent2D m_Extent{};

    VkSurfaceFormatKHR ChooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR ChoosePresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    VkExtent2D ChooseExtent(const VkSurfaceCapabilitiesKHR& capabilities);
};