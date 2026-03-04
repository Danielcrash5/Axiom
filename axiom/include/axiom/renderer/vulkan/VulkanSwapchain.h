#pragma once
#include "VulkanContext.h"
#include <vector>

class VulkanSwapchain {
public:
    VulkanSwapchain(VulkanContext* context);
    ~VulkanSwapchain();

    void Init();
    void Cleanup();
    void Recreate();

    VkSwapchainKHR GetSwapchain() const { return m_Swapchain; }
    VkFormat GetImageFormat() const { return m_ImageFormat; }
    VkExtent2D GetExtent() const { return m_Extent; }
    const std::vector<VkImage>& GetImages() const { return m_Images; }
    const std::vector<VkImageView>& GetImageViews() const { return m_ImageViews; }
    uint32_t GetImageCount() const { return static_cast<uint32_t>(m_Images.size()); }

private:
    void CreateSwapchain();
    void CreateImageViews();

    VulkanContext* m_Context = nullptr;
    VkSwapchainKHR m_Swapchain = VK_NULL_HANDLE;
    std::vector<VkImage> m_Images;
    std::vector<VkImageView> m_ImageViews;
    VkFormat m_ImageFormat = VK_FORMAT_UNDEFINED;
    VkExtent2D m_Extent = {0, 0};
};
