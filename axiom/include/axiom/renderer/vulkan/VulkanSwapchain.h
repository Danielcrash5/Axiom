#pragma once
#include "VulkanContext.h"
#include <vector>

class VulkanSwapchain {
public:
    VulkanSwapchain(VulkanContext* context);
    ~VulkanSwapchain();

    void Init();
    void Cleanup();

private:
    VulkanContext* m_Context = nullptr;
    VkSwapchainKHR m_Swapchain = VK_NULL_HANDLE;
    std::vector<VkImage> m_Images;
    VkFormat m_ImageFormat;
    VkExtent2D m_Extent;
};