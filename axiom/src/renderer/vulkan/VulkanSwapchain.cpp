#include "axiom/renderer/vulkan/VulkanSwapchain.h"
#include <stdexcept>
#include <algorithm>

VulkanSwapchain::VulkanSwapchain(VulkanContext* context)
    : m_Context(context) {
}

VulkanSwapchain::~VulkanSwapchain() {
    Cleanup();
}

void VulkanSwapchain::Init() {
    CreateSwapchain();
    CreateImageViews();
}

void VulkanSwapchain::Cleanup() {
    VkDevice device = m_Context->GetDevice();

    for (auto view : m_ImageViews) {
        if (view != VK_NULL_HANDLE)
            vkDestroyImageView(device, view, nullptr);
    }
    m_ImageViews.clear();

    if (m_Swapchain != VK_NULL_HANDLE) {
        vkDestroySwapchainKHR(device, m_Swapchain, nullptr);
        m_Swapchain = VK_NULL_HANDLE;
    }
    m_Images.clear();
}

void VulkanSwapchain::Recreate() {
    Cleanup();
    CreateSwapchain();
    CreateImageViews();
}

void VulkanSwapchain::CreateSwapchain() {
    VkSurfaceKHR surface = m_Context->GetSurface();
    VkPhysicalDevice physicalDevice = m_Context->GetPhysicalDevice();
    VkDevice device = m_Context->GetDevice();

    VkSurfaceCapabilitiesKHR capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);

    std::vector<VkSurfaceFormatKHR> formats(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, formats.data());

    // Prefer SRGB if available
    VkSurfaceFormatKHR surfaceFormat = formats[0];
    for (const auto& fmt : formats) {
        if (fmt.format == VK_FORMAT_B8G8R8A8_SRGB && fmt.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            surfaceFormat = fmt;
            break;
        }
    }

    // Handle special extent case (e.g. Wayland)
    VkExtent2D extent;
    if (capabilities.currentExtent.width != UINT32_MAX) {
        extent = capabilities.currentExtent;
    } else {
        int w, h;
        glfwGetFramebufferSize(m_Context->GetWindow(), &w, &h);
        extent.width  = std::clamp(static_cast<uint32_t>(w), capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        extent.height = std::clamp(static_cast<uint32_t>(h), capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
    }

    m_ImageFormat = surfaceFormat.format;
    m_Extent = extent;

    uint32_t imageCount = capabilities.minImageCount + 1;
    if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount)
        imageCount = capabilities.maxImageCount;

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = m_ImageFormat;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = m_Extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    uint32_t graphics = m_Context->GetGraphicsQueueFamilyIndex();
    uint32_t present = m_Context->GetPresentQueueFamilyIndex();

    if (graphics != present) {
        uint32_t indices[] = { graphics, present };
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = indices;
    } else {
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

void VulkanSwapchain::CreateImageViews() {
    m_ImageViews.resize(m_Images.size());

    for (size_t i = 0; i < m_Images.size(); i++) {
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = m_Images[i];
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = m_ImageFormat;
        viewInfo.components = { VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
                                VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY };
        viewInfo.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };

        if (vkCreateImageView(m_Context->GetDevice(), &viewInfo, nullptr, &m_ImageViews[i]) != VK_SUCCESS)
            throw std::runtime_error("Failed to create swapchain image view!");
    }
}
