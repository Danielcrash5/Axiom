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
    [[nodiscard]] RHIResult<void> attachSurface(void* nativeSurfaceHandle) override;

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
    VkSurfaceKHR m_surface = VK_NULL_HANDLE;

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
        VmaAllocation allocation = VK_NULL_HANDLE;
        VkImageView view = VK_NULL_HANDLE;
        TextureFormat format{};
        uint32_t width = 0, height = 0;
        uint32_t generation = 0;
        bool alive = false;
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

    // Staging-Buffer fuer Uploads, waechst bei Bedarf
    VkBuffer      m_stagingBuffer = VK_NULL_HANDLE;
    VmaAllocation m_stagingAllocation = VK_NULL_HANDLE;
    void*         m_stagingMapped = nullptr;
    uint64_t      m_stagingCapacity = 0;
};

} // namespace axiom::renderer::rhi::vulkan
