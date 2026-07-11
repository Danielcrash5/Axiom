#include <axiom/renderer/vulkan/VulkanBackend.h>
#include <cstring>
#include <iostream>
#include <optional>

namespace axiom::renderer::rhi::vulkan {

namespace {

VkFormat toVkFormat(TextureFormat format) {
    switch (format) {
        case TextureFormat::RGBA8Unorm:   return VK_FORMAT_R8G8B8A8_UNORM;
        case TextureFormat::BGRA8Unorm:   return VK_FORMAT_B8G8R8A8_UNORM;
        case TextureFormat::Depth32Float: return VK_FORMAT_D32_SFLOAT;
        case TextureFormat::R8Unorm:      return VK_FORMAT_R8_UNORM;
    }
    return VK_FORMAT_R8G8B8A8_UNORM;
}

uint32_t bytesPerPixel(TextureFormat format) {
    switch (format) {
        case TextureFormat::RGBA8Unorm:
        case TextureFormat::BGRA8Unorm:
        case TextureFormat::Depth32Float: return 4;
        case TextureFormat::R8Unorm:      return 1;
    }
    return 4;
}

VkBufferUsageFlags toVkBufferUsage(BufferUsage usage) {
    VkBufferUsageFlags flags = 0;
    if (usage & BufferUsage::Vertex)  flags |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    if (usage & BufferUsage::Index)   flags |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    if (usage & BufferUsage::Uniform) flags |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    if (usage & BufferUsage::Storage) flags |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    if (usage & BufferUsage::CopySrc) flags |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    if (usage & BufferUsage::CopyDst) flags |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    return flags;
}

VkImageUsageFlags toVkImageUsage(TextureUsage usage) {
    VkImageUsageFlags flags = 0;
    if (usage & TextureUsage::RenderTarget) flags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    if (usage & TextureUsage::Sampled)      flags |= VK_IMAGE_USAGE_SAMPLED_BIT;
    if (usage & TextureUsage::CopySrc)      flags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    if (usage & TextureUsage::CopyDst)      flags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    return flags;
}

#ifndef NDEBUG
VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT severity,
    VkDebugUtilsMessageTypeFlagsEXT,
    const VkDebugUtilsMessengerCallbackDataEXT* data,
    void*) {
    if (severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
        std::cerr << "[Vulkan] " << data->pMessage << "\n";
    }
    return VK_FALSE;
}
#endif

} // anonymous namespace

RHIResult<std::unique_ptr<VulkanBackend>> VulkanBackend::create() {
    auto backend = std::unique_ptr<VulkanBackend>(new VulkanBackend());

    // --- Instance ---
    VkApplicationInfo appInfo{ .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO };
    appInfo.pApplicationName = "Axiom";
    appInfo.apiVersion = VK_API_VERSION_1_3;

    std::vector<const char*> instanceLayers;
    std::vector<const char*> instanceExtensions;
#ifndef NDEBUG
    instanceLayers.push_back("VK_LAYER_KHRONOS_validation");
    instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

    VkInstanceCreateInfo instanceInfo{ .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
    instanceInfo.pApplicationInfo = &appInfo;
    instanceInfo.enabledLayerCount = static_cast<uint32_t>(instanceLayers.size());
    instanceInfo.ppEnabledLayerNames = instanceLayers.data();
    instanceInfo.enabledExtensionCount = static_cast<uint32_t>(instanceExtensions.size());
    instanceInfo.ppEnabledExtensionNames = instanceExtensions.data();

    if (vkCreateInstance(&instanceInfo, nullptr, &backend->m_instance) != VK_SUCCESS) {
        return std::unexpected(RHIError::Unknown);
    }

#ifndef NDEBUG
    // Extension-Funktionen lädt der SDK-Loader nicht automatisch,
    // Proc-Address-Lookup ist hier normal (kein volk-Ersatz nötig).
    auto createDebugMessenger = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
        vkGetInstanceProcAddr(backend->m_instance, "vkCreateDebugUtilsMessengerEXT"));
    if (createDebugMessenger) {
        VkDebugUtilsMessengerCreateInfoEXT debugInfo{
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT };
        debugInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
                                   | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        debugInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
                               | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
                               | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        debugInfo.pfnUserCallback = debugCallback;
        createDebugMessenger(backend->m_instance, &debugInfo, nullptr, &backend->m_debugMessenger);
    }
#endif

    // --- Physical Device Selection (manuell, kein vk-bootstrap) ---
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(backend->m_instance, &deviceCount, nullptr);
    if (deviceCount == 0) {
        return std::unexpected(RHIError::AdapterRequestFailed);
    }
    std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
    vkEnumeratePhysicalDevices(backend->m_instance, &deviceCount, physicalDevices.data());

    VkPhysicalDevice chosen = VK_NULL_HANDLE;
    uint32_t chosenGraphicsFamily = 0;

    for (auto candidate : physicalDevices) {
        VkPhysicalDeviceProperties props{};
        vkGetPhysicalDeviceProperties(candidate, &props);
        if (props.apiVersion < VK_API_VERSION_1_3) continue;

        VkPhysicalDeviceVulkan13Features features13{
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES };
        VkPhysicalDeviceVulkan12Features features12{
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES, .pNext = &features13 };
        VkPhysicalDeviceFeatures2 features2{
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2, .pNext = &features12 };
        vkGetPhysicalDeviceFeatures2(candidate, &features2);

        if (!features13.dynamicRendering || !features13.synchronization2 ||
            !features12.descriptorIndexing || !features12.runtimeDescriptorArray) {
            continue;
        }

        uint32_t familyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(candidate, &familyCount, nullptr);
        std::vector<VkQueueFamilyProperties> families(familyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(candidate, &familyCount, families.data());

        std::optional<uint32_t> graphicsFamily;
        for (uint32_t i = 0; i < familyCount; ++i) {
            if (families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) { graphicsFamily = i; break; }
        }
        if (!graphicsFamily) continue;

        bool isDiscrete = props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
        if (chosen == VK_NULL_HANDLE || isDiscrete) {
            chosen = candidate;
            chosenGraphicsFamily = *graphicsFamily;
            if (isDiscrete) break;
        }
    }

    if (chosen == VK_NULL_HANDLE) {
        return std::unexpected(RHIError::AdapterRequestFailed);
    }
    backend->m_physicalDevice = chosen;
    backend->m_graphicsQueueFamily = chosenGraphicsFamily;

    // --- Device ---
    float queuePriority = 1.0f;
    VkDeviceQueueCreateInfo queueInfo{ .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
    queueInfo.queueFamilyIndex = chosenGraphicsFamily;
    queueInfo.queueCount = 1;
    queueInfo.pQueuePriorities = &queuePriority;

    VkPhysicalDeviceVulkan13Features enable13{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES };
    enable13.dynamicRendering = VK_TRUE;
    enable13.synchronization2 = VK_TRUE;

    VkPhysicalDeviceVulkan12Features enable12{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES, .pNext = &enable13 };
    enable12.descriptorIndexing = VK_TRUE;
    enable12.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;
    enable12.descriptorBindingPartiallyBound = VK_TRUE;
    enable12.descriptorBindingVariableDescriptorCount = VK_TRUE;
    enable12.runtimeDescriptorArray = VK_TRUE;

    VkPhysicalDeviceFeatures2 enableFeatures2{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2, .pNext = &enable12 };

    VkDeviceCreateInfo deviceInfo{ .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
    deviceInfo.pNext = &enableFeatures2; // Features2-Chain statt pEnabledFeatures
    deviceInfo.queueCreateInfoCount = 1;
    deviceInfo.pQueueCreateInfos = &queueInfo;

    if (vkCreateDevice(chosen, &deviceInfo, nullptr, &backend->m_device) != VK_SUCCESS) {
        return std::unexpected(RHIError::DeviceRequestFailed);
    }
    vkGetDeviceQueue(backend->m_device, chosenGraphicsFamily, 0, &backend->m_graphicsQueue);

    // --- VMA ---
    VmaAllocatorCreateInfo allocatorInfo{};
    allocatorInfo.physicalDevice = backend->m_physicalDevice;
    allocatorInfo.device = backend->m_device;
    allocatorInfo.instance = backend->m_instance;
    allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_3;
    // Keine VmaVulkanFunctions nötig – VMA nutzt standardmäßig die
    // statisch gelinkten vk*-Funktionen des SDK-Loaders.

    if (vmaCreateAllocator(&allocatorInfo, &backend->m_allocator) != VK_SUCCESS) {
        return std::unexpected(RHIError::Unknown);
    }

    // --- Upload-Infrastruktur ---
    VkCommandPoolCreateInfo poolInfo{ .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = chosenGraphicsFamily;
    if (vkCreateCommandPool(backend->m_device, &poolInfo, nullptr, &backend->m_uploadCommandPool) != VK_SUCCESS) {
        return std::unexpected(RHIError::Unknown);
    }

    VkCommandBufferAllocateInfo cmdAllocInfo{ .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
    cmdAllocInfo.commandPool = backend->m_uploadCommandPool;
    cmdAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cmdAllocInfo.commandBufferCount = 1;
    if (vkAllocateCommandBuffers(backend->m_device, &cmdAllocInfo, &backend->m_uploadCommandBuffer) != VK_SUCCESS) {
        return std::unexpected(RHIError::Unknown);
    }

    VkFenceCreateInfo fenceInfo{ .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
    if (vkCreateFence(backend->m_device, &fenceInfo, nullptr, &backend->m_uploadFence) != VK_SUCCESS) {
        return std::unexpected(RHIError::Unknown);
    }

    return backend;
}

VulkanBackend::~VulkanBackend() {
    if (m_device == VK_NULL_HANDLE) return;

    vkDeviceWaitIdle(m_device);

    if (m_stagingBuffer != VK_NULL_HANDLE) {
        vmaDestroyBuffer(m_allocator, m_stagingBuffer, m_stagingAllocation);
    }
    for (auto& slot : m_buffers) {
        if (slot.alive) vmaDestroyBuffer(m_allocator, slot.buffer, slot.allocation);
    }
    for (auto& slot : m_textures) {
        if (slot.alive) {
            vkDestroyImageView(m_device, slot.view, nullptr);
            vmaDestroyImage(m_allocator, slot.image, slot.allocation);
        }
    }

    vkDestroyFence(m_device, m_uploadFence, nullptr);
    vkDestroyCommandPool(m_device, m_uploadCommandPool, nullptr);
    vmaDestroyAllocator(m_allocator);
    vkDestroyDevice(m_device, nullptr);

    if (m_debugMessenger != VK_NULL_HANDLE) {
        auto destroyDebugMessenger = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
            vkGetInstanceProcAddr(m_instance, "vkDestroyDebugUtilsMessengerEXT"));
        if (destroyDebugMessenger) destroyDebugMessenger(m_instance, m_debugMessenger, nullptr);
    }
    vkDestroyInstance(m_instance, nullptr);
}

// createBuffer, destroyBuffer, createTexture, destroyTexture,
// ensureStagingCapacity, submitImmediate, uploadBufferData, uploadTextureData
// bleiben INHALTLICH IDENTISCH zur letzten Version – die nutzten schon
// reines VkBuffer/VkImage/VmaAllocation, keine vkb-Typen. Kannst du 1:1
// aus meiner letzten Nachricht übernehmen.

std::unique_ptr<CommandList> VulkanBackend::createCommandList() {
    return nullptr; // TODO Phase 3
}

void VulkanBackend::submit(CommandList&) {
    // TODO Phase 3
}

} // namespace axiom::renderer::rhi::vulkan