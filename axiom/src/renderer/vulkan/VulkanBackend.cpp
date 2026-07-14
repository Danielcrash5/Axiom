#include <axiom/renderer/vulkan/VulkanBackend.h>
#include <cstring>
#include <iostream>
#include <optional>

namespace axiom::renderer::rhi::vulkan
{

    namespace
    {

        VkFormat toVkFormat(TextureFormat format)
        {
            switch (format)
            {
            case TextureFormat::RGBA8Unorm:
                return VK_FORMAT_R8G8B8A8_UNORM;
            case TextureFormat::BGRA8Unorm:
                return VK_FORMAT_B8G8R8A8_UNORM;
            case TextureFormat::Depth32Float:
                return VK_FORMAT_D32_SFLOAT;
            case TextureFormat::R8Unorm:
                return VK_FORMAT_R8_UNORM;
            }
            return VK_FORMAT_R8G8B8A8_UNORM;
        }

        uint32_t bytesPerPixel(TextureFormat format)
        {
            switch (format)
            {
            case TextureFormat::RGBA8Unorm:
            case TextureFormat::BGRA8Unorm:
            case TextureFormat::Depth32Float:
                return 4;
            case TextureFormat::R8Unorm:
                return 1;
            }
            return 4;
        }

        VkBufferUsageFlags toVkBufferUsage(BufferUsage usage)
        {
            VkBufferUsageFlags flags = 0;
            if (usage & BufferUsage::Vertex)
                flags |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
            if (usage & BufferUsage::Index)
                flags |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
            if (usage & BufferUsage::Uniform)
                flags |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
            if (usage & BufferUsage::Storage)
                flags |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
            if (usage & BufferUsage::CopySrc)
                flags |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
            if (usage & BufferUsage::CopyDst)
                flags |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
            return flags;
        }

        VkImageUsageFlags toVkImageUsage(TextureUsage usage)
        {
            VkImageUsageFlags flags = 0;
            if (usage & TextureUsage::RenderTarget)
                flags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
            if (usage & TextureUsage::Sampled)
                flags |= VK_IMAGE_USAGE_SAMPLED_BIT;
            if (usage & TextureUsage::CopySrc)
                flags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
            if (usage & TextureUsage::CopyDst)
                flags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
            return flags;
        }

#ifndef NDEBUG
        VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
            VkDebugUtilsMessageSeverityFlagBitsEXT severity,
            VkDebugUtilsMessageTypeFlagsEXT,
            const VkDebugUtilsMessengerCallbackDataEXT *data,
            void *)
        {
            if (severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
            {
                std::cerr << "[Vulkan] " << data->pMessage << "\n";
            }
            return VK_FALSE;
        }
#endif

    } // anonymous namespace

    RHIResult<std::unique_ptr<VulkanBackend>> VulkanBackend::create()
    {
        auto backend = std::unique_ptr<VulkanBackend>(new VulkanBackend());

        // --- Instance ---
        VkApplicationInfo appInfo{.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO};
        appInfo.pApplicationName = "Axiom";
        appInfo.apiVersion = VK_API_VERSION_1_3;

        std::vector<const char *> instanceLayers;
        std::vector<const char *> instanceExtensions;
#ifndef NDEBUG
        instanceLayers.push_back("VK_LAYER_KHRONOS_validation");
        instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

        VkInstanceCreateInfo instanceInfo{.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
        instanceInfo.pApplicationInfo = &appInfo;
        instanceInfo.enabledLayerCount = static_cast<uint32_t>(instanceLayers.size());
        instanceInfo.ppEnabledLayerNames = instanceLayers.data();
        instanceInfo.enabledExtensionCount = static_cast<uint32_t>(instanceExtensions.size());
        instanceInfo.ppEnabledExtensionNames = instanceExtensions.data();

        if (vkCreateInstance(&instanceInfo, nullptr, &backend->m_instance) != VK_SUCCESS)
        {
            return std::unexpected(RHIError::Unknown);
        }

#ifndef NDEBUG
        // Extension-Funktionen lädt der SDK-Loader nicht automatisch,
        // Proc-Address-Lookup ist hier normal (kein volk-Ersatz nötig).
        auto createDebugMessenger = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
            vkGetInstanceProcAddr(backend->m_instance, "vkCreateDebugUtilsMessengerEXT"));
        if (createDebugMessenger)
        {
            VkDebugUtilsMessengerCreateInfoEXT debugInfo{
                .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT};
            debugInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
            debugInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
            debugInfo.pfnUserCallback = debugCallback;
            createDebugMessenger(backend->m_instance, &debugInfo, nullptr, &backend->m_debugMessenger);
        }
#endif

        // --- Physical Device Selection (manuell, kein vk-bootstrap) ---
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(backend->m_instance, &deviceCount, nullptr);
        if (deviceCount == 0)
        {
            return std::unexpected(RHIError::AdapterRequestFailed);
        }
        std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
        vkEnumeratePhysicalDevices(backend->m_instance, &deviceCount, physicalDevices.data());

        VkPhysicalDevice chosen = VK_NULL_HANDLE;
        uint32_t chosenGraphicsFamily = 0;

        for (auto candidate : physicalDevices)
        {
            VkPhysicalDeviceProperties props{};
            vkGetPhysicalDeviceProperties(candidate, &props);
            if (props.apiVersion < VK_API_VERSION_1_3)
                continue;

            VkPhysicalDeviceVulkan13Features features13{
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES};
            VkPhysicalDeviceVulkan12Features features12{
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES, .pNext = &features13};
            VkPhysicalDeviceFeatures2 features2{
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2, .pNext = &features12};
            vkGetPhysicalDeviceFeatures2(candidate, &features2);

            if (!features13.dynamicRendering || !features13.synchronization2 ||
                !features12.descriptorIndexing || !features12.runtimeDescriptorArray)
            {
                continue;
            }

            uint32_t familyCount = 0;
            vkGetPhysicalDeviceQueueFamilyProperties(candidate, &familyCount, nullptr);
            std::vector<VkQueueFamilyProperties> families(familyCount);
            vkGetPhysicalDeviceQueueFamilyProperties(candidate, &familyCount, families.data());

            std::optional<uint32_t> graphicsFamily;
            for (uint32_t i = 0; i < familyCount; ++i)
            {
                if (families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
                {
                    graphicsFamily = i;
                    break;
                }
            }
            if (!graphicsFamily)
                continue;

            bool isDiscrete = props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
            if (chosen == VK_NULL_HANDLE || isDiscrete)
            {
                chosen = candidate;
                chosenGraphicsFamily = *graphicsFamily;
                if (isDiscrete)
                    break;
            }
        }

        if (chosen == VK_NULL_HANDLE)
        {
            return std::unexpected(RHIError::AdapterRequestFailed);
        }
        backend->m_physicalDevice = chosen;
        backend->m_graphicsQueueFamily = chosenGraphicsFamily;

        // --- Device ---
        float queuePriority = 1.0f;
        VkDeviceQueueCreateInfo queueInfo{.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO};
        queueInfo.queueFamilyIndex = chosenGraphicsFamily;
        queueInfo.queueCount = 1;
        queueInfo.pQueuePriorities = &queuePriority;

        VkPhysicalDeviceVulkan13Features enable13{
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES};
        enable13.dynamicRendering = VK_TRUE;
        enable13.synchronization2 = VK_TRUE;

        VkPhysicalDeviceVulkan12Features enable12{
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES, .pNext = &enable13};
        enable12.descriptorIndexing = VK_TRUE;
        enable12.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;
        enable12.descriptorBindingPartiallyBound = VK_TRUE;
        enable12.descriptorBindingVariableDescriptorCount = VK_TRUE;
        enable12.runtimeDescriptorArray = VK_TRUE;

        VkPhysicalDeviceFeatures2 enableFeatures2{
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2, .pNext = &enable12};

        VkDeviceCreateInfo deviceInfo{.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
        deviceInfo.pNext = &enableFeatures2; // Features2-Chain statt pEnabledFeatures
        deviceInfo.queueCreateInfoCount = 1;
        deviceInfo.pQueueCreateInfos = &queueInfo;

        if (vkCreateDevice(chosen, &deviceInfo, nullptr, &backend->m_device) != VK_SUCCESS)
        {
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

        if (vmaCreateAllocator(&allocatorInfo, &backend->m_allocator) != VK_SUCCESS)
        {
            return std::unexpected(RHIError::Unknown);
        }

        // --- Upload-Infrastruktur ---
        VkCommandPoolCreateInfo poolInfo{.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolInfo.queueFamilyIndex = chosenGraphicsFamily;
        if (vkCreateCommandPool(backend->m_device, &poolInfo, nullptr, &backend->m_uploadCommandPool) != VK_SUCCESS)
        {
            return std::unexpected(RHIError::Unknown);
        }

        VkCommandBufferAllocateInfo cmdAllocInfo{.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
        cmdAllocInfo.commandPool = backend->m_uploadCommandPool;
        cmdAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        cmdAllocInfo.commandBufferCount = 1;
        if (vkAllocateCommandBuffers(backend->m_device, &cmdAllocInfo, &backend->m_uploadCommandBuffer) != VK_SUCCESS)
        {
            return std::unexpected(RHIError::Unknown);
        }

        VkFenceCreateInfo fenceInfo{.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
        if (vkCreateFence(backend->m_device, &fenceInfo, nullptr, &backend->m_uploadFence) != VK_SUCCESS)
        {
            return std::unexpected(RHIError::Unknown);
        }

        VkCommandPoolCreateInfo graphPoolInfo{.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
        graphPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        graphPoolInfo.queueFamilyIndex = chosenGraphicsFamily;
        if (vkCreateCommandPool(backend->m_device, &graphPoolInfo, nullptr, &backend->m_graphCommandPool) != VK_SUCCESS)
        {
            return std::unexpected(RHIError::Unknown);
        }

        VkFenceCreateInfo graphFenceInfo{.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
        if (vkCreateFence(backend->m_device, &graphFenceInfo, nullptr, &backend->m_graphFence) != VK_SUCCESS)
        {
            return std::unexpected(RHIError::Unknown);
        }

        return backend;
    }

    VulkanBackend::~VulkanBackend()
    {
        if (m_device == VK_NULL_HANDLE)
            return;

        vkDeviceWaitIdle(m_device);

        if (m_stagingBuffer != VK_NULL_HANDLE)
        {
            vmaDestroyBuffer(m_allocator, m_stagingBuffer, m_stagingAllocation);
        }
        for (auto &slot : m_buffers)
        {
            if (slot.alive)
                vmaDestroyBuffer(m_allocator, slot.buffer, slot.allocation);
        }
        for (auto &slot : m_textures)
        {
            if (slot.alive)
            {
                vkDestroyImageView(m_device, slot.view, nullptr);
                vmaDestroyImage(m_allocator, slot.image, slot.allocation);
            }
        }

        vkDestroyFence(m_device, m_uploadFence, nullptr);
        vkDestroyCommandPool(m_device, m_uploadCommandPool, nullptr);
        vmaDestroyAllocator(m_allocator);
        vkDestroyDevice(m_device, nullptr);

        vkDestroyFence(m_device, m_graphFence, nullptr);
        vkDestroyCommandPool(m_device, m_graphCommandPool, nullptr);

        if (m_debugMessenger != VK_NULL_HANDLE)
        {
            auto destroyDebugMessenger = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
                vkGetInstanceProcAddr(m_instance, "vkDestroyDebugUtilsMessengerEXT"));
            if (destroyDebugMessenger)
                destroyDebugMessenger(m_instance, m_debugMessenger, nullptr);
        }
        vkDestroyInstance(m_instance, nullptr);
    }

    // createBuffer, destroyBuffer, createTexture, destroyTexture,
    // ensureStagingCapacity, submitImmediate, uploadBufferData, uploadTextureData
    // bleiben INHALTLICH IDENTISCH zur letzten Version – die nutzten schon
    // reines VkBuffer/VkImage/VmaAllocation, keine vkb-Typen. Kannst du 1:1
    // aus meiner letzten Nachricht übernehmen.

    VkImage VulkanBackend::nativeImage(TextureHandle handle) const
    {
        if (handle.index >= m_textures.size())
            return VK_NULL_HANDLE;
        auto &slot = m_textures[handle.index];
        if (!slot.alive || slot.generation != handle.generation)
            return VK_NULL_HANDLE;
        return slot.image;
    }

    VkBuffer VulkanBackend::nativeBuffer(BufferHandle handle) const
    {
        if (handle.index >= m_buffers.size())
            return VK_NULL_HANDLE;
        auto &slot = m_buffers[handle.index];
        if (!slot.alive || slot.generation != handle.generation)
            return VK_NULL_HANDLE;
        return slot.buffer;
    }

    std::unique_ptr<CommandList> VulkanBackend::createCommandList()
    {
        VkCommandBufferAllocateInfo allocInfo{.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
        allocInfo.commandPool = m_graphCommandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer commandBuffer;
        if (vkAllocateCommandBuffers(m_device, &allocInfo, &commandBuffer) != VK_SUCCESS)
        {
            return nullptr;
        }

        VkCommandBufferBeginInfo beginInfo{.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        vkBeginCommandBuffer(commandBuffer, &beginInfo);

        return std::make_unique<VulkanCommandList>(*this, commandBuffer);
    }

    void VulkanBackend::submit(CommandList &cmd)
    {
        auto &vkCmd = static_cast<VulkanCommandList &>(cmd);
        VkCommandBuffer commandBuffer = vkCmd.nativeHandle();

        vkEndCommandBuffer(commandBuffer);

        VkSubmitInfo submitInfo{.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO};
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        vkResetFences(m_device, 1, &m_graphFence);
        vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, m_graphFence);
        // Synchron/blockierend für Phase 2 – Frame-Pacing mit mehreren Fences
        // "in flight" kommt erst mit der Swapchain in Phase 8.
        vkWaitForFences(m_device, 1, &m_graphFence, VK_TRUE, UINT64_MAX);

        vkFreeCommandBuffers(m_device, m_graphCommandPool, 1, &commandBuffer);
    }

    namespace
    {
        VkFormat toVkVertexFormat(VertexFormat format)
        {
            switch (format)
            {
            case VertexFormat::Float32:
                return VK_FORMAT_R32_SFLOAT;
            case VertexFormat::Float32x2:
                return VK_FORMAT_R32G32_SFLOAT;
            case VertexFormat::Float32x3:
                return VK_FORMAT_R32G32B32_SFLOAT;
            case VertexFormat::Float32x4:
                return VK_FORMAT_R32G32B32A32_SFLOAT;
            case VertexFormat::Uint32:
                return VK_FORMAT_R32_UINT;
            case VertexFormat::Uint8x4Norm:
                return VK_FORMAT_R8G8B8A8_UNORM;
            }
            return VK_FORMAT_R32G32B32_SFLOAT;
        }

        VkPrimitiveTopology toVkTopology(PrimitiveTopology topo)
        {
            switch (topo)
            {
            case PrimitiveTopology::TriangleList:
                return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
            case PrimitiveTopology::LineList:
                return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
            case PrimitiveTopology::LineStrip:
                return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
            case PrimitiveTopology::PointList:
                return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
            }
            return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        }

        VkCullModeFlags toVkCullMode(CullMode cull)
        {
            switch (cull)
            {
            case CullMode::None:
                return VK_CULL_MODE_NONE;
            case CullMode::Front:
                return VK_CULL_MODE_FRONT_BIT;
            case CullMode::Back:
                return VK_CULL_MODE_BACK_BIT;
            }
            return VK_CULL_MODE_BACK_BIT;
        }

        VkShaderModule createShaderModule(VkDevice device, std::span<const uint32_t> spirv)
        {
            if (spirv.empty())
                return VK_NULL_HANDLE;
            VkShaderModuleCreateInfo info{.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
            info.codeSize = spirv.size() * sizeof(uint32_t);
            info.pCode = spirv.data();
            VkShaderModule module;
            if (vkCreateShaderModule(device, &info, nullptr, &module) != VK_SUCCESS)
                return VK_NULL_HANDLE;
            return module;
        }
    } // anonymous namespace

    RHIResult<BindGroupHandle> VulkanBackend::createBindGroup(const BindGroupDesc &desc)
    {
        // ... Pool-Erzeugung wie gehabt, ABER: Pool braucht
        // VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT, falls bindless ...

        // ... vkCreateDescriptorPool wie gehabt ...

        
        // ... vkAllocateDescriptorSets wie gehabt ...
    }

    RHIResult<PipelineHandle> VulkanBackend::createPipeline(const PipelineDesc &desc)
    {
        // --- Shader-Module ---
        VkShaderModule vertModule = createShaderModule(m_device, desc.shader.vertexSpirv);
        VkShaderModule fragModule = createShaderModule(m_device, desc.shader.pixelSpirv);
        if ((desc.shader.stages & ShaderStage::Vertex) && vertModule == VK_NULL_HANDLE)
        {
            return std::unexpected(RHIError::InvalidDescriptor);
        }
        if ((desc.shader.stages & ShaderStage::Pixel) && fragModule == VK_NULL_HANDLE)
        {
            if (vertModule != VK_NULL_HANDLE)
                vkDestroyShaderModule(m_device, vertModule, nullptr);
            return std::unexpected(RHIError::InvalidDescriptor);
        }

        std::vector<VkPipelineShaderStageCreateInfo> stages;
        if (vertModule != VK_NULL_HANDLE)
        {
            stages.push_back({.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                              .stage = VK_SHADER_STAGE_VERTEX_BIT,
                              .module = vertModule,
                              .pName = "main"});
        }
        if (fragModule != VK_NULL_HANDLE)
        {
            stages.push_back({.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                              .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
                              .module = fragModule,
                              .pName = "main"});
        }

        // --- Vertex-Input aus VertexLayout ---
        VkVertexInputBindingDescription binding{};
        binding.binding = 0;
        binding.stride = desc.shader.vertexLayout.stride;
        binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        std::vector<VkVertexInputAttributeDescription> attributes;
        for (auto &attr : desc.shader.vertexLayout.attributes)
        {
            attributes.push_back({.location = attr.location, .binding = 0, .format = toVkVertexFormat(attr.format), .offset = attr.offset});
        }

        VkPipelineVertexInputStateCreateInfo vertexInput{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};
        vertexInput.vertexBindingDescriptionCount = desc.shader.vertexLayout.stride > 0 ? 1 : 0;
        vertexInput.pVertexBindingDescriptions = &binding;
        vertexInput.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributes.size());
        vertexInput.pVertexAttributeDescriptions = attributes.data();

        VkPipelineInputAssemblyStateCreateInfo inputAssembly{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
        inputAssembly.topology = toVkTopology(desc.state.topology);

        // --- Dynamic State: Viewport/Scissor jedes Mal per CommandList gesetzt
        //     (negativer Viewport-Height für Y-up, siehe Abschnitt 1) ---
        VkPipelineViewportStateCreateInfo viewportState{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};
        viewportState.viewportCount = 1;
        viewportState.scissorCount = 1;

        std::array<VkDynamicState, 2> dynamicStates{VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
        VkPipelineDynamicStateCreateInfo dynamicState{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO};
        dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
        dynamicState.pDynamicStates = dynamicStates.data();

        VkPipelineRasterizationStateCreateInfo rasterizer{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizer.cullMode = toVkCullMode(desc.state.cull);
        rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE; // Standard bei rechtshändig
        rasterizer.lineWidth = 1.0f;

        VkPipelineMultisampleStateCreateInfo multisample{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
        multisample.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        VkPipelineDepthStencilStateCreateInfo depthStencil{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};
        depthStencil.depthTestEnable = desc.state.depthTest;
        depthStencil.depthWriteEnable = desc.state.depthWrite;
        depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;

        VkPipelineColorBlendAttachmentState blendAttachment{};
        blendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        if (desc.state.blend != BlendMode::None)
        {
            blendAttachment.blendEnable = VK_TRUE;
            blendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
            blendAttachment.dstColorBlendFactor = desc.state.blend == BlendMode::Additive
                                                      ? VK_BLEND_FACTOR_ONE
                                                      : VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
            blendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
            blendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
            blendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
            blendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
        }

        VkPipelineColorBlendStateCreateInfo colorBlend{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};
        colorBlend.attachmentCount = 1;
        colorBlend.pAttachments = &blendAttachment;

        // --- Pipeline-Layout aus BindGroupLayouts ---
        std::vector<VkDescriptorSetLayout> setLayouts;
        for (auto handle : desc.bindGroupLayouts)
        {
            setLayouts.push_back(nativeDescriptorSetLayout(handle)); // Helper, siehe unten
        }
        VkPipelineLayoutCreateInfo layoutInfo{.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
        layoutInfo.setLayoutCount = static_cast<uint32_t>(setLayouts.size());
        layoutInfo.pSetLayouts = setLayouts.data();

        VkPipelineLayout pipelineLayout;
        if (vkCreatePipelineLayout(m_device, &layoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
        {
            if (vertModule)
                vkDestroyShaderModule(m_device, vertModule, nullptr);
            if (fragModule)
                vkDestroyShaderModule(m_device, fragModule, nullptr);
            return std::unexpected(RHIError::Unknown);
        }

        // --- Dynamic Rendering statt VkRenderPass ---
        VkFormat colorFormat = toVkFormat(desc.colorTargetFormat);
        VkPipelineRenderingCreateInfo renderingInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO};
        renderingInfo.colorAttachmentCount = 1;
        renderingInfo.pColorAttachmentFormats = &colorFormat;
        VkFormat depthFormat = VK_FORMAT_UNDEFINED;
        if (desc.depthTargetFormat)
        {
            depthFormat = toVkFormat(*desc.depthTargetFormat);
            renderingInfo.depthAttachmentFormat = depthFormat;
        }

        VkGraphicsPipelineCreateInfo pipelineInfo{.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};
        pipelineInfo.pNext = &renderingInfo;
        pipelineInfo.stageCount = static_cast<uint32_t>(stages.size());
        pipelineInfo.pStages = stages.data();
        pipelineInfo.pVertexInputState = &vertexInput;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisample;
        pipelineInfo.pDepthStencilState = &depthStencil;
        pipelineInfo.pColorBlendState = &colorBlend;
        pipelineInfo.pDynamicState = &dynamicState;
        pipelineInfo.layout = pipelineLayout;

        VkPipeline pipeline;
        VkResult result = vkCreateGraphicsPipelines(m_device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline);

        if (vertModule)
            vkDestroyShaderModule(m_device, vertModule, nullptr);
        if (fragModule)
            vkDestroyShaderModule(m_device, fragModule, nullptr);

        if (result != VK_SUCCESS)
        {
            vkDestroyPipelineLayout(m_device, pipelineLayout, nullptr);
            return std::unexpected(RHIError::Unknown);
        }

        uint32_t index;
        if (!m_freePipelineSlots.empty())
        {
            index = m_freePipelineSlots.back();
            m_freePipelineSlots.pop_back();
            m_pipelines[index].generation += 1;
        }
        else
        {
            index = static_cast<uint32_t>(m_pipelines.size());
            m_pipelines.push_back(PipelineSlot{});
            m_pipelines[index].generation = 1;
        }
        m_pipelines[index].pipeline = pipeline;
        m_pipelines[index].layout = pipelineLayout;
        m_pipelines[index].alive = true;

        return PipelineHandle{index, m_pipelines[index].generation};
    }

    namespace
    {
        VkDescriptorType toVkDescriptorType(BindingType type)
        {
            switch (type)
            {
            case BindingType::UniformBuffer:
                return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            case BindingType::StorageBuffer:
                return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            case BindingType::SampledTexture:
                return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
            case BindingType::Sampler:
                return VK_DESCRIPTOR_TYPE_SAMPLER;
            }
            return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        }
        VkShaderStageFlags toVkShaderStageFlags(ShaderStage stage)
        {
            VkShaderStageFlags flags = 0;
            if (stage & ShaderStage::Vertex)
                flags |= VK_SHADER_STAGE_VERTEX_BIT;
            if (stage & ShaderStage::Pixel)
                flags |= VK_SHADER_STAGE_FRAGMENT_BIT;
            if (stage & ShaderStage::Compute)
                flags |= VK_SHADER_STAGE_COMPUTE_BIT;
            return flags;
        }
    } // anonymous namespace

    RHIResult<BindGroupLayoutHandle> VulkanBackend::createBindGroupLayout(const BindGroupLayoutDesc &desc)
    {
        std::vector<VkDescriptorSetLayoutBinding> bindings;
        std::vector<VkDescriptorBindingFlags> bindingFlags;
        bool hasBindless = false;

        for (auto &entry : desc.entries)
        {
            VkDescriptorSetLayoutBinding binding{};
            binding.binding = entry.binding;
            binding.descriptorType = toVkDescriptorType(entry.type);
            binding.stageFlags = toVkShaderStageFlags(entry.visibility);
            binding.descriptorCount = entry.bindless ? entry.bindlessMaxCount : 1;
            bindings.push_back(binding);

            VkDescriptorBindingFlags flags = 0;
            if (entry.bindless)
            {
                flags = VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT | VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;
                hasBindless = true;
            }
            bindingFlags.push_back(flags);
        }

        VkDescriptorSetLayoutBindingFlagsCreateInfo flagsInfo{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO};
        flagsInfo.bindingCount = static_cast<uint32_t>(bindingFlags.size());
        flagsInfo.pBindingFlags = bindingFlags.data();

        VkDescriptorSetLayoutCreateInfo layoutInfo{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
        layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
        layoutInfo.pBindings = bindings.data();
        if (hasBindless)
        {
            layoutInfo.pNext = &flagsInfo;
            layoutInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
        }

        VkDescriptorSetLayout layout;
        if (vkCreateDescriptorSetLayout(m_device, &layoutInfo, nullptr, &layout) != VK_SUCCESS)
        {
            return std::unexpected(RHIError::Unknown);
        }

        std::vector<VkDescriptorSetLayoutBinding> bindings;
        for (auto &entry : desc.entries)
        {
            bindings.push_back({.binding = entry.binding,
                                .descriptorType = toVkDescriptorType(entry.type),
                                .descriptorCount = 1,
                                .stageFlags = toVkShaderStageFlags(entry.visibility)});
        }

        VkDescriptorSetLayoutCreateInfo layoutInfo{.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
        layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
        layoutInfo.pBindings = bindings.data();

        VkDescriptorSetLayout layout;
        if (vkCreateDescriptorSetLayout(m_device, &layoutInfo, nullptr, &layout) != VK_SUCCESS)
        {
            return std::unexpected(RHIError::Unknown);
        }

        uint32_t index;
        if (!m_freeBindGroupLayoutSlots.empty())
        {
            index = m_freeBindGroupLayoutSlots.back();
            m_freeBindGroupLayoutSlots.pop_back();
            m_bindGroupLayouts[index].generation += 1;
        }
        else
        {
            index = static_cast<uint32_t>(m_bindGroupLayouts.size());
            m_bindGroupLayouts.push_back(BindGroupLayoutSlot{});
            m_bindGroupLayouts[index].generation = 1;
        }
        m_bindGroupLayouts[index].layout = layout;
        m_bindGroupLayouts[index].alive = true;

        return BindGroupLayoutHandle{index, m_bindGroupLayouts[index].generation};
    }

    RHIResult<BindGroupHandle> VulkanBackend::createBindGroup(const BindGroupDesc &desc)
    {
        VkDescriptorSetLayout layout = nativeDescriptorSetLayout(desc.layout);
        if (layout == VK_NULL_HANDLE)
            return std::unexpected(RHIError::InvalidDescriptor);

        // Ein Pool pro BindGroup ist ineffizient, reicht aber für Phase 3 –
        // ein zentraler wachsender Pool ist eine spätere Optimierung.
        VkDescriptorPoolSize poolSizes[] = {
            {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 8},
            {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 8},
            {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 8},
            {VK_DESCRIPTOR_TYPE_SAMPLER, 8},
        };
        VkDescriptorPoolCreateInfo poolInfo{.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
        poolInfo.maxSets = 1;
        poolInfo.poolSizeCount = 4;
        poolInfo.pPoolSizes = poolSizes;

        VkDescriptorPoolCreateInfo poolInfo{.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
        poolInfo.maxSets = 1;
        poolInfo.poolSizeCount = 4;
        poolInfo.pPoolSizes = poolSizes;
        if (desc.bindlessCount)
        {
            poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
        }

        VkDescriptorPool pool;
        if (vkCreateDescriptorPool(m_device, &poolInfo, nullptr, &pool) != VK_SUCCESS)
        {
            return std::unexpected(RHIError::Unknown);
        }

        VkDescriptorSetVariableDescriptorCountAllocateInfo variableCountInfo{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO};
        uint32_t count = desc.bindlessCount.value_or(0);
        variableCountInfo.descriptorSetCount = 1;
        variableCountInfo.pDescriptorCounts = &count;

        VkDescriptorSetAllocateInfo allocInfo{.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
        allocInfo.descriptorPool = pool;
        allocInfo.descriptorSetCount = 1;
        allocInfo.pSetLayouts = &layout;
        if (desc.bindlessCount)
        {
            allocInfo.pNext = &variableCountInfo;
        }

        VkDescriptorSetAllocateInfo allocInfo{.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
        allocInfo.descriptorPool = pool;
        allocInfo.descriptorSetCount = 1;
        allocInfo.pSetLayouts = &layout;

        VkDescriptorSet set;
        if (vkAllocateDescriptorSets(m_device, &allocInfo, &set) != VK_SUCCESS)
        {
            vkDestroyDescriptorPool(m_device, pool, nullptr);
            return std::unexpected(RHIError::Unknown);
        }

        std::vector<VkWriteDescriptorSet> writes;
        std::vector<VkDescriptorBufferInfo> bufferInfos;
        bufferInfos.reserve(desc.entries.size());
        for (auto &entry : desc.entries)
        {
            if (entry.buffer)
            {
                bufferInfos.push_back({nativeBuffer(*entry.buffer), 0, VK_WHOLE_SIZE});
                writes.push_back({.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                                  .dstSet = set,
                                  .dstBinding = entry.binding,
                                  .descriptorCount = 1,
                                  .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                  .pBufferInfo = &bufferInfos.back()});
            }
            // texture/sampler-Bindings folgen analog, hier gekürzt (gleiche Fälle
            // wie Bindless-Heap in Punkt 13, dort ausführlicher).
        }
        vkUpdateDescriptorSets(m_device, static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);

        uint32_t index;
        if (!m_freeBindGroupSlots.empty())
        {
            index = m_freeBindGroupSlots.back();
            m_freeBindGroupSlots.pop_back();
            m_bindGroups[index].generation += 1;
        }
        else
        {
            index = static_cast<uint32_t>(m_bindGroups.size());
            m_bindGroups.push_back(BindGroupSlot{});
            m_bindGroups[index].generation = 1;
        }
        m_bindGroups[index].pool = pool;
        m_bindGroups[index].set = set;
        m_bindGroups[index].alive = true;

        return BindGroupHandle{index, m_bindGroups[index].generation};
    }

} // namespace axiom::renderer::rhi::vulkan