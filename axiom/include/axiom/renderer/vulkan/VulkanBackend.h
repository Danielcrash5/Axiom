/**
 * @file axiom/include/axiom/renderer/vulkan/VulkanBackend.h
 * @brief Declares the public interface for the vulkan backend.
 */

#pragma once
#include <vulkan/vulkan.h>
#include <vma/vk_mem_alloc.h>
#include <vector>
#include <span>
#include <functional>
#include <optional>
#include <utility>
#include <axiom/renderer/rhi/IRHIBackend.h>
#include <axiom/renderer/rhi/PipelineDesc.h>
#include <axiom/renderer/rhi/BindGroup.h>
#include <axiom/renderer/rhi/Sampler.h>

namespace axiom::renderer::rhi::vulkan {

class VulkanCommandList; // fwd

/**
 * @brief Provides the vulkan backend functionality.
 * @tparam VulkanBackend Template type used by this type.
 */
class VulkanBackend final : public IRHIBackend {
public:
    /**
     * @brief Creates the requested resource.
     * @param requiredInstanceExtensions The required instance extensions used by this operation.
     * @return The result of the operation.
     */
    [[nodiscard]] static RHIResult<std::unique_ptr<VulkanBackend>> create(
        std::span<const char* const> requiredInstanceExtensions = {});
    /**
     * @brief Releases resources owned by VulkanBackend.
     */
    ~VulkanBackend() override;

    /**
     * @brief Creates an instance of vulkan backend.
     * @param VulkanBackend The vulkan backend used by this operation.
     */
    VulkanBackend(const VulkanBackend&) = delete;
    VulkanBackend& operator=(const VulkanBackend&) = delete;

    // --- IRHIBackend: Buffers/Textures (Phase 1) ---
    /**
     * @brief Creates buffer.
     * @param BufferDesc The buffer desc used by this operation.
     * @return The result of the operation.
     */
    RHIResult<BufferHandle> createBuffer(const BufferDesc&) override;
    /**
     * @brief Creates texture.
     * @param TextureDesc The texture desc used by this operation.
     * @return The result of the operation.
     */
    RHIResult<TextureHandle> createTexture(const TextureDesc&) override;
    /**
     * @brief Destroys the buffer and releases its resources.
     */
    void destroyBuffer(BufferHandle) override;
    /**
     * @brief Destroys the texture and releases its resources.
     */
    void destroyTexture(TextureHandle) override;

    /**
     * @brief Uploads data to a buffer.
     * @param offset The offset used by this operation.
     * @param data The data used by this operation.
     */
    RHIResult<void> uploadBufferData(BufferHandle, uint64_t offset,
                                      std::span<const std::byte> data) override;
    /**
     * @brief Uploads data to a texture.
     * @param TextureUploadDesc The texture upload desc used by this operation.
     * @param data The data used by this operation.
     */
    RHIResult<void> uploadTextureData(TextureHandle, const TextureUploadDesc&,
                                       std::span<const std::byte> data) override;

    // --- IRHIBackend: CommandList (Phase 2) ---
    /**
     * @brief Creates command list.
     * @return The result of the operation.
     */
    std::unique_ptr<CommandList> createCommandList() override;
    /**
     * @brief Submits the collected work for execution.
     */
    void submit(CommandList&) override;

    // --- IRHIBackend: Fenster-Integration ---
    /**
     * @brief Returns the native instance handle.
     * @return The result of the operation.
     */
    [[nodiscard]] void* nativeInstanceHandle() const override;
    /**
     * @brief Creates surface.
     * @param nativeSurfaceHandle The native surface handle used by this operation.
     * @return The result of the operation.
     */
    [[nodiscard]] RHIResult<SurfaceHandle> createSurface(void* nativeSurfaceHandle) override;
    /**
     * @brief Destroys the surface and releases its resources.
     */
    void destroySurface(SurfaceHandle) override;

    // --- IRHIBackend: Pipelines/BindGroups (Phase 3) ---
    /**
     * @brief Creates pipeline.
     * @param PipelineDesc The pipeline desc used by this operation.
     * @return The result of the operation.
     */
    RHIResult<PipelineHandle> createPipeline(const PipelineDesc&) override;
    /**
     * @brief Destroys the pipeline and releases its resources.
     */
    void destroyPipeline(PipelineHandle) override;
    /**
     * @brief Creates bind group layout.
     * @param BindGroupLayoutDesc The bind group layout desc used by this operation.
     * @return The result of the operation.
     */
    RHIResult<BindGroupLayoutHandle> createBindGroupLayout(const BindGroupLayoutDesc&) override;
    /**
     * @brief Creates bind group.
     * @param BindGroupDesc The bind group desc used by this operation.
     * @return The result of the operation.
     */
    RHIResult<BindGroupHandle> createBindGroup(const BindGroupDesc&) override;
    /**
     * @brief Updates bind group.
     * @param target The target used by this operation.
     * @param entries The entries used by this operation.
     */
    RHIResult<void> updateBindGroup(BindGroupHandle target, std::span<const BindGroupEntry> entries) override;
    /**
     * @brief Destroys the bind group layout.
     */
    void destroyBindGroupLayout(BindGroupLayoutHandle) override;
    /**
     * @brief Destroys the bind group.
     */
    void destroyBindGroup(BindGroupHandle) override;

    /**
     * @brief Creates sampler.
     * @param SamplerDesc The sampler desc used by this operation.
     * @return The result of the operation.
     */
    RHIResult<SamplerHandle> createSampler(const SamplerDesc&) override;
    /**
     * @brief Destroys the sampler.
     */
    void destroySampler(SamplerHandle) override;

    // --- IRHIBackend: Swapchain/Present ---
    /**
     * @brief Creates swapchain.
     * @param SwapchainDesc The swapchain desc used by this operation.
     * @return The result of the operation.
     */
    RHIResult<SwapchainHandle> createSwapchain(const SwapchainDesc&) override;
    /**
     * @brief Destroys the swapchain and releases its resources.
     */
    void destroySwapchain(SwapchainHandle) override;
    /**
     * @brief Acquires the next image from the swapchain.
     * @return The result of the operation.
     */
    RHIResult<AcquiredImage> acquireNextImage(SwapchainHandle) override;
    /**
     * @brief Presents the rendered image to the display.
     * @param imageIndex The image index used by this operation.
     */
    RHIResult<void> present(SwapchainHandle, uint32_t imageIndex) override;
    /**
     * @brief Sets swapchain vsync.
     * @param enabled The enabled used by this operation.
     */
    [[nodiscard]] RHIResult<void> setSwapchainVsync(SwapchainHandle,
                                                     bool enabled) override;
    /**
     * @brief Returns the swapchain format.
     * @return The result of the operation.
     */
    TextureFormat swapchainFormat(SwapchainHandle) const override;
    /**
     * @brief Returns the swapchain extent.
     * @return The result of the operation.
     */
    std::pair<uint32_t, uint32_t> swapchainExtent(SwapchainHandle) const override;

    // --- Interne Helfer, von VulkanCommandList genutzt (kein IRHIBackend-Bestandteil) ---
    /**
     * @brief Returns the native image.
     * @param handle The handle used by this operation.
     * @return The result of the operation.
     */
    [[nodiscard]] VkImage nativeImage(TextureHandle handle) const;
    /**
     * @brief Returns the native image view.
     * @param handle The handle used by this operation.
     * @return The result of the operation.
     */
    [[nodiscard]] VkImageView nativeImageView(TextureHandle handle) const;
    /**
     * @brief Returns the native buffer.
     * @param handle The handle used by this operation.
     * @return The result of the operation.
     */
    [[nodiscard]] VkBuffer nativeBuffer(BufferHandle handle) const;
    /**
     * @brief Returns the native extent.
     * @param handle The handle used by this operation.
     * @return The result of the operation.
     */
    [[nodiscard]] std::pair<uint32_t, uint32_t> nativeExtent(TextureHandle handle) const;

    /** @brief Stores the data for the native pipeline. */
    struct NativePipeline { VkPipeline pipeline = VK_NULL_HANDLE; VkPipelineLayout layout = VK_NULL_HANDLE; };
    /**
     * @brief Returns the native pipeline.
     * @param handle The handle used by this operation.
     * @return The result of the operation.
     */
    [[nodiscard]] NativePipeline nativePipeline(PipelineHandle handle) const;
    /**
     * @brief Returns the native descriptor set layout.
     * @param handle The handle used by this operation.
     * @return The result of the operation.
     */
    [[nodiscard]] VkDescriptorSetLayout nativeDescriptorSetLayout(BindGroupLayoutHandle handle) const;
    /**
     * @brief Returns the native descriptor set.
     * @param handle The handle used by this operation.
     * @return The result of the operation.
     */
    [[nodiscard]] VkDescriptorSet nativeDescriptorSet(BindGroupHandle handle) const;
    /**
     * @brief Returns the native sampler.
     * @param handle The handle used by this operation.
     * @return The result of the operation.
     */
    [[nodiscard]] VkSampler nativeSampler(SamplerHandle handle) const;
    /**
     * @brief Returns the native surface.
     * @param handle The handle used by this operation.
     * @return The result of the operation.
     */
    [[nodiscard]] VkSurfaceKHR nativeSurface(SurfaceHandle handle) const;

    // Rohe Vulkan-Handles fuer Integrationen, die selbst gegen Vulkan
    // sprechen muessen (z.B. imgui_impl_vulkan). Nur nutzen, wenn ein
    // Vulkan-spezifischer Layer das explizit braucht - der Rest der Engine
    // geht ueber IRHIBackend.
    /** @brief Stores the data for the native context. */
    struct NativeContext {
        /** @brief Stores the instance. */
        VkInstance       instance = VK_NULL_HANDLE;
        /** @brief Stores the physical device. */
        VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
        /** @brief Stores the device. */
        VkDevice         device = VK_NULL_HANDLE;
        /** @brief Stores the graphics queue. */
        VkQueue          graphicsQueue = VK_NULL_HANDLE;
        /** @brief Stores the graphics queue family. */
        uint32_t         graphicsQueueFamily = 0;
    };
    /**
     * @brief Returns the native context.
     * @return The result of the operation.
     */
    [[nodiscard]] NativeContext nativeContext() const {
        return {m_instance, m_physicalDevice, m_device, m_graphicsQueue,
                m_graphicsQueueFamily};
    }

private:
    /**
     * @brief Creates an instance of vulkan backend.
     */
    VulkanBackend() = default;

    /**
     * @brief Performs the ensure staging capacity operation.
     * @param sizeBytes The size bytes used by this operation.
     */
    RHIResult<void> ensureStagingCapacity(uint64_t sizeBytes);
    /**
     * @brief Submits immediate.
     * @param record The record used by this operation.
     */
    RHIResult<void> submitImmediate(const std::function<void(VkCommandBuffer)>& record);

    /** @brief Stores the instance. */
    VkInstance m_instance = VK_NULL_HANDLE;
    /** @brief Stores the debug messenger. */
    VkDebugUtilsMessengerEXT m_debugMessenger = VK_NULL_HANDLE;
    /** @brief Stores the physical device. */
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
    /** @brief Stores the device. */
    VkDevice m_device = VK_NULL_HANDLE;
    /** @brief Stores the graphics queue. */
    VkQueue m_graphicsQueue = VK_NULL_HANDLE;
    /** @brief Stores the graphics queue family. */
    uint32_t m_graphicsQueueFamily = 0;
    /** @brief Stores the allocator. */
    VmaAllocator m_allocator = VK_NULL_HANDLE;

    // Upload-Infrastruktur (Phase 1, synchrone Immediate-Uploads)
    /** @brief Stores the upload command pool. */
    VkCommandPool   m_uploadCommandPool = VK_NULL_HANDLE;
    /** @brief Stores the upload command buffer. */
    VkCommandBuffer m_uploadCommandBuffer = VK_NULL_HANDLE;
    /** @brief Stores the upload fence. */
    VkFence         m_uploadFence = VK_NULL_HANDLE;

    // Command-Recording-Infrastruktur fuer den RenderGraph (Phase 2)
    /** @brief Stores the graph command pool. */
    VkCommandPool m_graphCommandPool = VK_NULL_HANDLE;
    /** @brief Stores the graph fence. */
    VkFence       m_graphFence = VK_NULL_HANDLE;

    // --- Slot-Pools (generation-basiert, siehe Handle<Tag>) ---
    /** @brief Stores the data for the buffer slot. */
    struct BufferSlot {
        /** @brief Stores the buffer. */
        VkBuffer buffer = VK_NULL_HANDLE;
        /** @brief Stores the allocation. */
        VmaAllocation allocation = VK_NULL_HANDLE;
        /** @brief Stores the size. */
        uint64_t size = 0;
        /** @brief Stores the generation. */
        uint32_t generation = 0;
        /** @brief Indicates whether the object is alive. */
        bool alive = false;
    };
    /** @brief Stores the data for the texture slot. */
    struct TextureSlot {
        /** @brief Stores the image. */
        VkImage image = VK_NULL_HANDLE;
        /** @brief Stores the allocation. */
        VmaAllocation allocation = VK_NULL_HANDLE; // VK_NULL_HANDLE bei externallyOwnedImage
        /** @brief Stores the view. */
        VkImageView view = VK_NULL_HANDLE;
        /** @brief Stores the format. */
        TextureFormat format{};
        /** @brief Stores the width. */
        uint32_t width = 0, height = 0;
        /** @brief Stores the generation. */
        uint32_t generation = 0;
        /** @brief Indicates whether the object is alive. */
        bool alive = false;
        // true fuer Swapchain-Images: VkImage gehoert der Swapchain (wird bei
        // vkDestroySwapchainKHR mitzerstoert), wir besitzen nur den ImageView.
        // destroyTexture() darf in diesem Fall NICHT vmaDestroyImage aufrufen.
        /** @brief Stores the externally owned image. */
        bool externallyOwnedImage = false;
    };
    /** @brief Stores the data for the pipeline slot. */
    struct PipelineSlot {
        /** @brief Stores the pipeline. */
        VkPipeline pipeline = VK_NULL_HANDLE;
        /** @brief Stores the layout. */
        VkPipelineLayout layout = VK_NULL_HANDLE;
        /** @brief Stores the generation. */
        uint32_t generation = 0;
        /** @brief Indicates whether the object is alive. */
        bool alive = false;
    };
    /** @brief Stores the data for the bind group layout slot. */
    struct BindGroupLayoutSlot {
        /** @brief Stores the layout. */
        VkDescriptorSetLayout layout = VK_NULL_HANDLE;
        /** @brief Stores the generation. */
        uint32_t generation = 0;
        /** @brief Indicates whether the object is alive. */
        bool alive = false;
    };
    /** @brief Stores the data for the bind group slot. */
    struct BindGroupSlot {
        /** @brief Stores the pool. */
        VkDescriptorPool pool = VK_NULL_HANDLE; // ein Pool pro BindGroup (Phase 3)
        /** @brief Stores the set. */
        VkDescriptorSet set = VK_NULL_HANDLE;
        /** @brief Stores the generation. */
        uint32_t generation = 0;
        /** @brief Indicates whether the object is alive. */
        bool alive = false;
    };
    /** @brief Stores the data for the sampler slot. */
    struct SamplerSlot {
        /** @brief Stores the sampler. */
        VkSampler sampler = VK_NULL_HANDLE;
        /** @brief Stores the generation. */
        uint32_t generation = 0;
        /** @brief Indicates whether the object is alive. */
        bool alive = false;
    };
    /** @brief Stores the data for the surface slot. */
    struct SurfaceSlot {
        /** @brief Stores the surface. */
        VkSurfaceKHR surface = VK_NULL_HANDLE;
        /** @brief Stores the generation. */
        uint32_t generation = 0;
        /** @brief Indicates whether the object is alive. */
        bool alive = false;
    };
    /** @brief Stores the data for the swapchain slot. */
    struct SwapchainSlot {
        /** @brief Stores the swapchain. */
        VkSwapchainKHR swapchain = VK_NULL_HANDLE;
        /** @brief Stores the surface. */
        VkSurfaceKHR surface = VK_NULL_HANDLE; // nicht besessen, nur referenziert
        /** @brief Stores the format. */
        VkFormat format = VK_FORMAT_UNDEFINED;
        /** @brief Stores the RHI format. */
        TextureFormat rhiFormat{};
        /** @brief Stores the width. */
        uint32_t width = 0, height = 0;
        /** @brief Stores the vsync. */
        bool vsync = true;
        // Ein stabiler TextureHandle PRO Swapchain-Image, einmal bei
        // (Re-)Erzeugung angelegt - siehe Swapchain.h fuer die Begruendung.
        /** @brief Stores the image textures. */
        std::vector<TextureHandle> imageTextures;
        // Ein Fence fuer acquireNextImage() - vollstaendig synchron, siehe
        // IRHIBackend::acquireNextImage()-Kommentar.
        /** @brief Stores the acquire fence. */
        VkFence acquireFence = VK_NULL_HANDLE;
        /** @brief Stores the generation. */
        uint32_t generation = 0;
        /** @brief Indicates whether the object is alive. */
        bool alive = false;
    };

    // Baut/erneuert die eigentliche VkSwapchainKHR + Image-Views fuer einen
    // bestehenden Slot - genutzt von createSwapchain() UND intern von
    // acquireNextImage() bei OUT_OF_DATE/SUBOPTIMAL.
    /**
     * @brief Performs the recreate swapchain internal operation.
     * @param slot The slot used by this operation.
     * @param requestedWidth The requested width used by this operation.
     * @param requestedHeight The requested height used by this operation.
     */
    RHIResult<void> recreateSwapchainInternal(SwapchainSlot& slot, uint32_t requestedWidth,
                                               uint32_t requestedHeight);

    /** @brief Stores the buffers. */
    std::vector<BufferSlot>  m_buffers;
    /** @brief Stores the free buffer slots. */
    std::vector<uint32_t>    m_freeBufferSlots;
    /** @brief Stores the textures. */
    std::vector<TextureSlot> m_textures;
    /** @brief Stores the free texture slots. */
    std::vector<uint32_t>    m_freeTextureSlots;
    /** @brief Stores the pipelines. */
    std::vector<PipelineSlot> m_pipelines;
    /** @brief Stores the free pipeline slots. */
    std::vector<uint32_t>     m_freePipelineSlots;
    /** @brief Stores the bind group layouts. */
    std::vector<BindGroupLayoutSlot> m_bindGroupLayouts;
    /** @brief Stores the free bind group layout slots. */
    std::vector<uint32_t>            m_freeBindGroupLayoutSlots;
    /** @brief Stores the bind groups. */
    std::vector<BindGroupSlot> m_bindGroups;
    /** @brief Stores the free bind group slots. */
    std::vector<uint32_t>      m_freeBindGroupSlots;
    /** @brief Stores the samplers. */
    std::vector<SamplerSlot> m_samplers;
    /** @brief Stores the free sampler slots. */
    std::vector<uint32_t>    m_freeSamplerSlots;
    /** @brief Stores the surfaces. */
    std::vector<SurfaceSlot> m_surfaces;
    /** @brief Stores the free surface slots. */
    std::vector<uint32_t>    m_freeSurfaceSlots;
    /** @brief Stores the swapchains. */
    std::vector<SwapchainSlot> m_swapchains;
    /** @brief Stores the free swapchain slots. */
    std::vector<uint32_t>      m_freeSwapchainSlots;

    // Staging-Buffer fuer Uploads, waechst bei Bedarf
    /** @brief Stores the staging buffer. */
    VkBuffer      m_stagingBuffer = VK_NULL_HANDLE;
    /** @brief Stores the staging allocation. */
    VmaAllocation m_stagingAllocation = VK_NULL_HANDLE;
    /** @brief Stores the staging mapped. */
    void*         m_stagingMapped = nullptr;
    /** @brief Stores the staging capacity. */
    uint64_t      m_stagingCapacity = 0;
};

} // namespace axiom::renderer::rhi::vulkan
