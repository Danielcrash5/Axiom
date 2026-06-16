#include <volk.h>
#include <renderer/RendererRHI.h>

namespace axiom {

    // Das private Innenleben des RHI Command Buffers
    struct CommandBufferImpl {
        VkCommandBuffer nativeCommandBuffer = VK_NULL_HANDLE;
    };

    CommandBuffer::CommandBuffer() {
        m_impl = new CommandBufferImpl();
    }

    CommandBuffer::~CommandBuffer() {
        delete m_impl;
    }

    void CommandBuffer::bind_vertex_buffer(BufferHandle buf, uint64_t offset) {
        // TODO: Implementierung sobald das Buffer-System steht
        // VkBuffer vkBuf = get_vulkan_buffer(buf);
        // vkCmdBindVertexBuffers(m_impl->nativeCommandBuffer, 0, 1, &vkBuf, &offset);
    }

    void CommandBuffer::bind_index_buffer(BufferHandle buf, uint64_t offset) {
        // TODO: Implementierung analog zu Vertex Buffer
        // vkCmdBindIndexBuffer(m_impl->nativeCommandBuffer, get_vulkan_buffer(buf), offset, VK_INDEX_TYPE_UINT32);
    }

    void CommandBuffer::draw_indexed_indirect(BufferHandle indirectBuffer, uint64_t offset, uint32_t drawCount) {
        // TODO: vkCmdDrawIndexedIndirect aufrufen
    }

    void CommandBuffer::push_constants(const void* data, uint32_t size) {
        // TODO: vkCmdPushConstants aufrufen (benötigt das aktuelle VkPipelineLayout)
    }

} // namespace Axiom