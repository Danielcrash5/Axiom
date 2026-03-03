#include "axiom/renderer/vulkan/VulkanContext.h"
#include "axiom/core/Logger.h"
#include <stdexcept>
#include <cstring>

VulkanContext::VulkanContext(GLFWwindow* window)
    : m_Window(window) {

    PickQueueFamilies(window);
}

VulkanContext::~VulkanContext() {
    Shutdown();
}

// Find graphics & present queue families
void VulkanContext::PickQueueFamilies(GLFWwindow* window) {
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalDevice, &queueFamilyCount, nullptr);

    if (queueFamilyCount == 0)
        throw std::runtime_error("No queue families found on the physical device!");

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalDevice, &queueFamilyCount, queueFamilies.data());

    int i = 0;
    for (const auto& queueFamily : queueFamilies) {
        // Check for graphics capability
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            m_Indices.graphicsFamily = i;

        // Check for present support
        VkBool32 presentSupport = VK_FALSE;
        vkGetPhysicalDeviceSurfaceSupportKHR(m_PhysicalDevice, i, /*surface*/nullptr, &presentSupport);
        // Achtung: Hier musst du die Surface-Instanz übergeben!
        // z.B. über m_Surface, die du nach glfwCreateWindowSurface speicherst.
        // Für jetzt, Platzhalter:

        if (presentSupport)
            m_Indices.presentFamily = i;

        if (m_Indices.IsComplete())
            break;
        i++;
    }

    if (!m_Indices.IsComplete())
        throw std::runtime_error("Failed to find required queue families (graphics + present)!");
}

void VulkanContext::Init() {
    CreateInstance();
    SetupDebugMessenger();
    PickPhysicalDevice();
    CreateLogicalDevice();
}

void VulkanContext::Shutdown() {
    if (m_Device != VK_NULL_HANDLE) {
        vkDeviceWaitIdle(m_Device);
        vkDestroyDevice(m_Device, nullptr);
    }

    DestroyDebugMessenger();

    if (m_Instance != VK_NULL_HANDLE)
        vkDestroyInstance(m_Instance, nullptr);
}

// ---------------- Instance & Debug Messenger ----------------

void VulkanContext::CreateInstance() {
    if (m_EnableValidationLayers && !CheckValidationLayerSupport())
        throw std::runtime_error("Validation layers requested but not available!");

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Axiom Engine";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "Axiom Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_3;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    auto extensions = GetRequiredExtensions();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    if (m_EnableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(m_ValidationLayers.size());
        createInfo.ppEnabledLayerNames = m_ValidationLayers.data();

        debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        debugCreateInfo.messageSeverity =
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        debugCreateInfo.messageType =
            VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        debugCreateInfo.pfnUserCallback = DebugCallback;
        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
    } else {
        createInfo.enabledLayerCount = 0;
        createInfo.pNext = nullptr;
    }

    if (vkCreateInstance(&createInfo, nullptr, &m_Instance) != VK_SUCCESS)
        throw std::runtime_error("Failed to create Vulkan instance!");
}

void VulkanContext::SetupDebugMessenger() {
    if (!m_EnableValidationLayers) return;

    VkDebugUtilsMessengerCreateInfoEXT createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity =
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType =
        VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = DebugCallback;

    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(m_Instance,
                                                                          "vkCreateDebugUtilsMessengerEXT");
    if (!func || func(m_Instance, &createInfo, nullptr, &m_DebugMessenger) != VK_SUCCESS)
        throw std::runtime_error("Failed to set up debug messenger!");
}

void VulkanContext::DestroyDebugMessenger() {
    if (!m_DebugMessenger) return;

    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(m_Instance,
                                                                           "vkDestroyDebugUtilsMessengerEXT");
    if (func)
        func(m_Instance, m_DebugMessenger, nullptr);

    m_DebugMessenger = VK_NULL_HANDLE;
}

// ---------------- Physical + Logical Device ----------------

void VulkanContext::PickPhysicalDevice() {
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(m_Instance, &deviceCount, nullptr);
    if (deviceCount == 0)
        throw std::runtime_error("Failed to find GPUs with Vulkan support!");

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(m_Instance, &deviceCount, devices.data());

    m_PhysicalDevice = devices[0]; // erstmal der erste
}

void VulkanContext::CreateLogicalDevice() {
    VkDeviceQueueCreateInfo queueCreateInfo{};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = 0; // TODO: Query korrekt
    queueCreateInfo.queueCount = 1;
    float priority = 1.0f;
    queueCreateInfo.pQueuePriorities = &priority;

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = 1;
    createInfo.pQueueCreateInfos = &queueCreateInfo;

    if (m_EnableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(m_ValidationLayers.size());
        createInfo.ppEnabledLayerNames = m_ValidationLayers.data();
    } else {
        createInfo.enabledLayerCount = 0;
    }

    if (vkCreateDevice(m_PhysicalDevice, &createInfo, nullptr, &m_Device) != VK_SUCCESS)
        throw std::runtime_error("Failed to create logical device!");
}

// ---------------- Helpers ----------------

bool VulkanContext::CheckValidationLayerSupport() {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char* layerName : m_ValidationLayers) {
        bool found = false;
        for (const auto& layerProps : availableLayers) {
            if (strcmp(layerName, layerProps.layerName) == 0) {
                found = true;
                break;
            }
        }
        if (!found) return false;
    }
    return true;
}

std::vector<const char*> VulkanContext::GetRequiredExtensions() {
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
    if (m_EnableValidationLayers)
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

    return extensions;
}

VKAPI_ATTR VkBool32 VKAPI_CALL VulkanContext::DebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT       messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT              messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData) {
    AXIOM_INFO("[VULKAN VALIDATION] {}", pCallbackData->pMessage);
    return VK_FALSE;
}