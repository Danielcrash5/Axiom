#pragma once

#include <vector>

#include <Volk/volk.h>

namespace axiom {
    class VulkanContext;
    class VulkanPhysicalDevice;
    class VulkanDevice;

    class VulkanSwapchain {
    public:
        VulkanSwapchain() = default;
        ~VulkanSwapchain() = default;

        void Create(
            const VulkanContext& context,
            const VulkanPhysicalDevice& physicalDevice,
            const VulkanDevice& device,
            uint32_t width,
            uint32_t height);

        void Destroy();

        void Recreate(
            const VulkanContext& context,
            const VulkanPhysicalDevice& physicalDevice,
            const VulkanDevice& device,
            uint32_t width,
            uint32_t height);

        VkSwapchainKHR GetHandle() const {
            return m_Swapchain;
        }

        VkFormat GetImageFormat() const {
            return m_ImageFormat;
        }

        VkExtent2D GetExtent() const {
            return m_Extent;
        }

        const std::vector<VkImage>& GetImages() const {
            return m_Images;
        }

        const std::vector<VkImageView>& GetImageViews() const {
            return m_ImageViews;
        }

    private:
        VkSwapchainKHR m_Swapchain = VK_NULL_HANDLE;

        VkFormat m_ImageFormat = VK_FORMAT_UNDEFINED;
        VkExtent2D m_Extent {};

        std::vector<VkImage> m_Images;
        std::vector<VkImageView> m_ImageViews;
    };
}