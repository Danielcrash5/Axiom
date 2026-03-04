#include "axiom/renderer/vulkan/VulkanSwapchain.h"
#include <stdexcept>

VulkanSwapchain::VulkanSwapchain(VulkanContext* context)
    : m_Context(context) {
}

VulkanSwapchain::~VulkanSwapchain() {
    Cleanup();
}

void VulkanSwapchain::Init() {
    VkSurfaceKHR surface = m_Context->GetSurface();
    VkPhysicalDevice physicalDevice = m_Context->GetPhysicalDevice();
    VkDevice device = m_Context->GetDevice();

    VkSurfaceCapabilitiesKHR capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);

    std::vector<VkSurfaceFormatKHR> formats(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, formats.data());

    VkSurfaceFormatKHR surfaceFormat = formats[0];

    VkExtent2D extent = capabilities.currentExtent;

    uint32_t imageCount = capabilities.minImageCount + 1;
    if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount)
        imageCount = capabilities.maxImageCount;

    VkSwapchainCreateInfoKHR createInfo {};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    uint32_t graphics = m_Context->GetGraphicsQueueFamilyIndex();
    uint32_t present = m_Context->GetPresentQueueFamilyIndex();

    if (graphics != present) {
        uint32_t indices[] = { graphics, present };
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = indices;
    }
    else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    createInfo.preTransform = capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR;
    createInfo.clipped = VK_TRUE;

    if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &m_Swapchain) != VK_SUCCESS)
        throw std::runtime_error("Failed to create swapchain!");

    uint32_t actualImageCount;
    vkGetSwapchainImagesKHR(device, m_Swapchain, &actualImageCount, nullptr);

    m_Images.resize(actualImageCount);
    vkGetSwapchainImagesKHR(device, m_Swapchain, &actualImageCount, m_Images.data());
}

void VulkanSwapchain::Cleanup() {
    if (m_Swapchain)
        vkDestroySwapchainKHR(m_Context->GetDevice(), m_Swapchain, nullptr);
}