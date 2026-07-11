#pragma once
#include <vulkan/vulkan.h>
#include <vma/vk_mem_alloc.h>
#include <vector>
#include <span>
#include <functional>
#include <axiom/renderer/rhi/IRHIBackend.h>

namespace axiom::renderer::rhi::vulkan {

class VulkanBackend final : public IRHIBackend {
public:
    [[nodiscard]] static RHIResult<std::unique_ptr<VulkanBackend>> create();
    ~VulkanBackend() override;

    VulkanBackend(const VulkanBackend&) = delete;
    VulkanBackend& operator=(const VulkanBackend&) = delete;

    RHIResult<BufferHandle> createBuffer(const BufferDesc&) override;
    RHIResult<TextureHandle> createTexture(const TextureDesc&) override;
    void destroyBuffer(BufferHandle) override;
    void destroyTexture(TextureHandle) override;

    RHIResult<void> uploadBufferData(BufferHandle, uint64_t offset,
                                      std::span<const std::byte> data) override;
    RHIResult<void> uploadTextureData(TextureHandle, const TextureUploadDesc&,
                                       std::span<const std::byte> data) override;

    std::unique_ptr<CommandList> createCommandList() override;
    void submit(CommandList&) override;

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

    VkCommandPool   m_uploadCommandPool = VK_NULL_HANDLE;
    VkCommandBuffer m_uploadCommandBuffer = VK_NULL_HANDLE;
    VkFence         m_uploadFence = VK_NULL_HANDLE;

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

    std::vector<BufferSlot>  m_buffers;
    std::vector<uint32_t>    m_freeBufferSlots;
    std::vector<TextureSlot> m_textures;
    std::vector<uint32_t>    m_freeTextureSlots;

    VkBuffer      m_stagingBuffer = VK_NULL_HANDLE;
    VmaAllocation m_stagingAllocation = VK_NULL_HANDLE;
    void*         m_stagingMapped = nullptr;
    uint64_t      m_stagingCapacity = 0;
};

} // namespace axiom::renderer::rhi::vulkan