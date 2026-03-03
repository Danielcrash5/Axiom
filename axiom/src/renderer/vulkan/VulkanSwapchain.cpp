#include "axiom/renderer/vulkan/VulkanSwapchain.h"
#include "axiom/renderer/vulkan/VulkanUtils.h"
#include "axiom/core/Logger.h"

VulkanSwapchain::VulkanSwapchain(VulkanContext* context, GLFWwindow* window)
    : m_Context(context), m_Window(window) {}

VulkanSwapchain::~VulkanSwapchain() {
    Cleanup();
}

void VulkanSwapchain::Init() {
    if (!m_Context)
        throw std::runtime_error("VulkanContext is null!");
    if (!m_Window)
        throw std::runtime_error("GLFWwindow is null!");

    // ---- Create Surface ----
    VkSurfaceKHR surface;
    if (glfwCreateWindowSurface(m_Context->GetInstance(), m_Window, nullptr, &surface) != VK_SUCCESS)
        throw std::runtime_error("Failed to create Vulkan window surface!");

    // ---- Check if queue supports presenting ----
    VkBool32 presentSupported = VK_FALSE;
    vkGetPhysicalDeviceSurfaceSupportKHR(m_Context->GetPhysicalDevice(),
                                         m_Context->GetGraphicsQueueFamilyIndex(),
                                         surface, &presentSupported);
    if (!presentSupported)
        throw std::runtime_error("Selected queue family does not support presenting to surface!");

    // ---- Query surface capabilities ----
    VkSurfaceCapabilitiesKHR capabilities;
    VKCheck(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_Context->GetPhysicalDevice(), surface, &capabilities),
            "Failed to get surface capabilities!");

    // ---- Query formats and present modes ----
    uint32_t formatCount;
    VKCheck(vkGetPhysicalDeviceSurfaceFormatsKHR(m_Context->GetPhysicalDevice(), surface, &formatCount, nullptr),
            "Failed to get surface formats!");
    if (formatCount == 0)
        throw std::runtime_error("No supported surface formats found!");

    std::vector<VkSurfaceFormatKHR> formats(formatCount);
    VKCheck(vkGetPhysicalDeviceSurfaceFormatsKHR(m_Context->GetPhysicalDevice(), surface, &formatCount, formats.data()),
            "Failed to get surface formats data!");

    uint32_t presentModeCount;
    VKCheck(vkGetPhysicalDeviceSurfacePresentModesKHR(m_Context->GetPhysicalDevice(), surface, &presentModeCount, nullptr),
            "Failed to get present modes!");
    std::vector<VkPresentModeKHR> presentModes(presentModeCount);
    VKCheck(vkGetPhysicalDeviceSurfacePresentModesKHR(m_Context->GetPhysicalDevice(), surface, &presentModeCount, presentModes.data()),
            "Failed to get present mode data!");

    // ---- Choose format, present mode and extent ----
    VkSurfaceFormatKHR surfaceFormat = ChooseSurfaceFormat(formats);
    VkPresentModeKHR presentMode = ChoosePresentMode(presentModes);
    VkExtent2D extent = ChooseExtent(capabilities);

    // ---- Determine image count ----
    uint32_t imageCount = capabilities.minImageCount + 1;
    if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount)
        imageCount = capabilities.maxImageCount;

    // ---- Create Swapchain ----
    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.preTransform = capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(m_Context->GetDevice(), &createInfo, nullptr, &m_Swapchain) != VK_SUCCESS)
        throw std::runtime_error("Failed to create swapchain!");

    // ---- Retrieve images ----
    uint32_t actualImageCount = 0;
    VKCheck(vkGetSwapchainImagesKHR(m_Context->GetDevice(), m_Swapchain, &actualImageCount, nullptr),
            "Failed to get swapchain image count!");
    m_Images.resize(actualImageCount);
    VKCheck(vkGetSwapchainImagesKHR(m_Context->GetDevice(), m_Swapchain, &actualImageCount, m_Images.data()),
            "Failed to get swapchain images!");

    m_ImageFormat = surfaceFormat.format;
    m_Extent = extent;

    //AXIOM_INFO("Swapchain created with {} images, format {}, extent {}x{}",
             //  m_Images.size(), m_ImageFormat, m_Extent.width, m_Extent.height);
    AXIOM_INFO("Swapchain created.");
}

// ---- Cleanup ----
void VulkanSwapchain::Cleanup() {
    if (m_Swapchain != VK_NULL_HANDLE) {
        vkDestroySwapchainKHR(m_Context->GetDevice(), m_Swapchain, nullptr);
        m_Swapchain = VK_NULL_HANDLE;
        m_Images.clear();
    }
}

// ---- Helpers ----
VkSurfaceFormatKHR VulkanSwapchain::ChooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
    for (const auto& f : availableFormats) {
        if (f.format == VK_FORMAT_B8G8R8A8_SRGB && f.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            return f;
    }
    return availableFormats[0];
}

VkPresentModeKHR VulkanSwapchain::ChoosePresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
    for (const auto& mode : availablePresentModes) {
        if (mode == VK_PRESENT_MODE_MAILBOX_KHR)
            return mode;
    }
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D VulkanSwapchain::ChooseExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
    if (capabilities.currentExtent.width != UINT32_MAX)
        return capabilities.currentExtent;
    return {1280, 720}; // fallback
}