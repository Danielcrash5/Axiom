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

class VulkanBackend final : public IRHIBackend {
public:
    [[nodiscard]] static RHIResult<std::unique_ptr<VulkanBackend>> create(
        std::span<const char* const> requiredInstanceExtensions = {});
    ~VulkanBackend() override;

    VulkanBackend(const VulkanBackend&) = delete;
    VulkanBackend& operator=(const VulkanBackend&) = delete;

    // --- IRHIBackend: Buffers/Textures (Phase 1) ---
    RHIResult<BufferHandle> createBuffer(const BufferDesc&) override;
    RHIResult<TextureHandle> createTexture(const TextureDesc&) override;
    void destroyBuffer(BufferHandle) override;
    void destroyTexture(TextureHandle) override;

    RHIResult<void> uploadBufferData(BufferHandle, uint64_t offset,
                                      std::span<const std::byte> data) override;
    RHIResult<void> uploadTextureData(TextureHandle, const TextureUploadDesc&,
                                       std::span<const std::byte> data) override;

    // --- IRHIBackend: CommandList (Phase 2) ---
    std::unique_ptr<CommandList> createCommandList() override;
    void submit(CommandList&) override;

    // --- IRHIBackend: Fenster-Integration ---
    [[nodiscard]] void* nativeInstanceHandle() const override;
    [[nodiscard]] RHIResult<SurfaceHandle> createSurface(void* nativeSurfaceHandle) override;
    void destroySurface(SurfaceHandle) override;

    // --- IRHIBackend: Pipelines/BindGroups (Phase 3) ---
    RHIResult<PipelineHandle> createPipeline(const PipelineDesc&) override;
    void destroyPipeline(PipelineHandle) override;
    RHIResult<BindGroupLayoutHandle> createBindGroupLayout(const BindGroupLayoutDesc&) override;
    RHIResult<BindGroupHandle> createBindGroup(const BindGroupDesc&) override;
    RHIResult<void> updateBindGroup(BindGroupHandle target, std::span<const BindGroupEntry> entries) override;
    void destroyBindGroupLayout(BindGroupLayoutHandle) override;
    void destroyBindGroup(BindGroupHandle) override;

    RHIResult<SamplerHandle> createSampler(const SamplerDesc&) override;
    void destroySampler(SamplerHandle) override;

    // --- IRHIBackend: Swapchain/Present ---
    RHIResult<SwapchainHandle> createSwapchain(const SwapchainDesc&) override;
    void destroySwapchain(SwapchainHandle) override;
    RHIResult<AcquiredImage> acquireNextImage(SwapchainHandle) override;
    RHIResult<void> present(SwapchainHandle, uint32_t imageIndex) override;
    [[nodiscard]] RHIResult<void> setSwapchainVsync(SwapchainHandle,
                                                     bool enabled) override;
    TextureFormat swapchainFormat(SwapchainHandle) const override;
    std::pair<uint32_t, uint32_t> swapchainExtent(SwapchainHandle) const override;

    // --- Interne Helfer, von VulkanCommandList genutzt (kein IRHIBackend-Bestandteil) ---
    [[nodiscard]] VkImage nativeImage(TextureHandle handle) const;
    [[nodiscard]] VkImageView nativeImageView(TextureHandle handle) const;
    [[nodiscard]] VkBuffer nativeBuffer(BufferHandle handle) const;
    [[nodiscard]] std::pair<uint32_t, uint32_t> nativeExtent(TextureHandle handle) const;

    struct NativePipeline { VkPipeline pipeline = VK_NULL_HANDLE; VkPipelineLayout layout = VK_NULL_HANDLE; };
    [[nodiscard]] NativePipeline nativePipeline(PipelineHandle handle) const;
    [[nodiscard]] VkDescriptorSetLayout nativeDescriptorSetLayout(BindGroupLayoutHandle handle) const;
    [[nodiscard]] VkDescriptorSet nativeDescriptorSet(BindGroupHandle handle) const;
    [[nodiscard]] VkSampler nativeSampler(SamplerHandle handle) const;
    [[nodiscard]] VkSurfaceKHR nativeSurface(SurfaceHandle handle) const;

    // Rohe Vulkan-Handles fuer Integrationen, die selbst gegen Vulkan
    // sprechen muessen (z.B. imgui_impl_vulkan). Nur nutzen, wenn ein
    // Vulkan-spezifischer Layer das explizit braucht - der Rest der Engine
    // geht ueber IRHIBackend.
    struct NativeContext {
        VkInstance       instance = VK_NULL_HANDLE;
        VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
        VkDevice         device = VK_NULL_HANDLE;
        VkQueue          graphicsQueue = VK_NULL_HANDLE;
        uint32_t         graphicsQueueFamily = 0;
    };
    [[nodiscard]] NativeContext nativeContext() const {
        return {m_instance, m_physicalDevice, m_device, m_graphicsQueue,
                m_graphicsQueueFamily};
    }

private:
    VulkanBackend() = default;

    RHIResult<void> ensureStagingCapacity(uint64_t sizeBytes);
    RHIResult<void> submitImmediate(const std::function<void(VkCommandBuffer)>& record);

    VkInstance m_instance = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT m_debugMessenger = VK_NULL_HANDLE;
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
    VkDevice m_device = VK_NULL_HANDLE;
    VkQueue m_graphicsQueue = VK_NULL_HANDLE;
    uint32_t m_graphicsQueueFamily = 0;
    VmaAllocator m_allocator = VK_NULL_HANDLE;

    // Upload-Infrastruktur (Phase 1, synchrone Immediate-Uploads)
    VkCommandPool   m_uploadCommandPool = VK_NULL_HANDLE;
    VkCommandBuffer m_uploadCommandBuffer = VK_NULL_HANDLE;
    VkFence         m_uploadFence = VK_NULL_HANDLE;

    // Command-Recording-Infrastruktur fuer den RenderGraph (Phase 2)
    VkCommandPool m_graphCommandPool = VK_NULL_HANDLE;
    VkFence       m_graphFence = VK_NULL_HANDLE;

    // --- Slot-Pools (generation-basiert, siehe Handle<Tag>) ---
    struct BufferSlot {
        VkBuffer buffer = VK_NULL_HANDLE;
        VmaAllocation allocation = VK_NULL_HANDLE;
        uint64_t size = 0;
        uint32_t generation = 0;
        bool alive = false;
    };
    struct TextureSlot {
        VkImage image = VK_NULL_HANDLE;
        VmaAllocation allocation = VK_NULL_HANDLE; // VK_NULL_HANDLE bei externallyOwnedImage
        VkImageView view = VK_NULL_HANDLE;
        TextureFormat format{};
        uint32_t width = 0, height = 0;
        uint32_t generation = 0;
        bool alive = false;
        // true fuer Swapchain-Images: VkImage gehoert der Swapchain (wird bei
        // vkDestroySwapchainKHR mitzerstoert), wir besitzen nur den ImageView.
        // destroyTexture() darf in diesem Fall NICHT vmaDestroyImage aufrufen.
        bool externallyOwnedImage = false;
    };
    struct PipelineSlot {
        VkPipeline pipeline = VK_NULL_HANDLE;
        VkPipelineLayout layout = VK_NULL_HANDLE;
        uint32_t generation = 0;
        bool alive = false;
    };
    struct BindGroupLayoutSlot {
        VkDescriptorSetLayout layout = VK_NULL_HANDLE;
        uint32_t generation = 0;
        bool alive = false;
    };
    struct BindGroupSlot {
        VkDescriptorPool pool = VK_NULL_HANDLE; // ein Pool pro BindGroup (Phase 3)
        VkDescriptorSet set = VK_NULL_HANDLE;
        uint32_t generation = 0;
        bool alive = false;
    };
    struct SamplerSlot {
        VkSampler sampler = VK_NULL_HANDLE;
        uint32_t generation = 0;
        bool alive = false;
    };
    struct SurfaceSlot {
        VkSurfaceKHR surface = VK_NULL_HANDLE;
        uint32_t generation = 0;
        bool alive = false;
    };
    struct SwapchainSlot {
        VkSwapchainKHR swapchain = VK_NULL_HANDLE;
        VkSurfaceKHR surface = VK_NULL_HANDLE; // nicht besessen, nur referenziert
        VkFormat format = VK_FORMAT_UNDEFINED;
        TextureFormat rhiFormat{};
        uint32_t width = 0, height = 0;
        bool vsync = true;
        // Ein stabiler TextureHandle PRO Swapchain-Image, einmal bei
        // (Re-)Erzeugung angelegt - siehe Swapchain.h fuer die Begruendung.
        std::vector<TextureHandle> imageTextures;
        // Ein Fence fuer acquireNextImage() - vollstaendig synchron, siehe
        // IRHIBackend::acquireNextImage()-Kommentar.
        VkFence acquireFence = VK_NULL_HANDLE;
        uint32_t generation = 0;
        bool alive = false;
    };

    // Baut/erneuert die eigentliche VkSwapchainKHR + Image-Views fuer einen
    // bestehenden Slot - genutzt von createSwapchain() UND intern von
    // acquireNextImage() bei OUT_OF_DATE/SUBOPTIMAL.
    RHIResult<void> recreateSwapchainInternal(SwapchainSlot& slot, uint32_t requestedWidth,
                                               uint32_t requestedHeight);

    std::vector<BufferSlot>  m_buffers;
    std::vector<uint32_t>    m_freeBufferSlots;
    std::vector<TextureSlot> m_textures;
    std::vector<uint32_t>    m_freeTextureSlots;
    std::vector<PipelineSlot> m_pipelines;
    std::vector<uint32_t>     m_freePipelineSlots;
    std::vector<BindGroupLayoutSlot> m_bindGroupLayouts;
    std::vector<uint32_t>            m_freeBindGroupLayoutSlots;
    std::vector<BindGroupSlot> m_bindGroups;
    std::vector<uint32_t>      m_freeBindGroupSlots;
    std::vector<SamplerSlot> m_samplers;
    std::vector<uint32_t>    m_freeSamplerSlots;
    std::vector<SurfaceSlot> m_surfaces;
    std::vector<uint32_t>    m_freeSurfaceSlots;
    std::vector<SwapchainSlot> m_swapchains;
    std::vector<uint32_t>      m_freeSwapchainSlots;

    // Staging-Buffer fuer Uploads, waechst bei Bedarf
    VkBuffer      m_stagingBuffer = VK_NULL_HANDLE;
    VmaAllocation m_stagingAllocation = VK_NULL_HANDLE;
    void*         m_stagingMapped = nullptr;
    uint64_t      m_stagingCapacity = 0;
};

} // namespace axiom::renderer::rhi::vulkan
