#include <Volk/volk.h>
#include <axiom/renderer/RendererRHI.h>
#include <stdexcept>

namespace axiom {
	struct CommandBufferImpl {
		VkCommandBuffer nativeCommandBuffer = VK_NULL_HANDLE;
		GraphicsDevice* deviceContext = nullptr;
	};

	CommandBuffer::CommandBuffer() {
		m_impl = new CommandBufferImpl();
	}

	CommandBuffer::~CommandBuffer() {
		delete m_impl;
	}

	void CommandBuffer::set_native_handle(void* cmdBuffer, GraphicsDevice* device) {
		m_impl->nativeCommandBuffer = static_cast<VkCommandBuffer>(cmdBuffer);
		m_impl->deviceContext = device;
	}

	void CommandBuffer::bind_vertex_buffer(BufferHandle buf, uint64_t offset) {
		if (!m_impl || m_impl->nativeCommandBuffer == VK_NULL_HANDLE || !m_impl->deviceContext) return;

		void* rawBuffer = m_impl->deviceContext->get_native_buffer_handle(buf);
		if (!rawBuffer) return;

		VkBuffer vkBuffer = static_cast<VkBuffer>(rawBuffer);
		VkDeviceSize vkOffset = offset;

		vkCmdBindVertexBuffers(m_impl->nativeCommandBuffer, 0, 1, &vkBuffer, &vkOffset);
	}

	void CommandBuffer::bind_index_buffer(BufferHandle buf, uint64_t offset) {
		if (!m_impl || m_impl->nativeCommandBuffer == VK_NULL_HANDLE || !m_impl->deviceContext) return;

		void* rawBuffer = m_impl->deviceContext->get_native_buffer_handle(buf);
		if (!rawBuffer) return;

		VkBuffer vkBuffer = static_cast<VkBuffer>(rawBuffer);
		vkCmdBindIndexBuffer(m_impl->nativeCommandBuffer, vkBuffer, offset, VK_INDEX_TYPE_UINT32);
	}

	void CommandBuffer::draw_indexed_indirect(BufferHandle indirectBuffer, uint64_t offset, uint32_t drawCount) {
		if (!m_impl || m_impl->nativeCommandBuffer == VK_NULL_HANDLE || !m_impl->deviceContext) return;

		void* rawBuffer = m_impl->deviceContext->get_native_buffer_handle(indirectBuffer);
		if (!rawBuffer) return;

		VkBuffer vkIndirectBuf = static_cast<VkBuffer>(rawBuffer);

		vkCmdDrawIndexedIndirect(
			m_impl->nativeCommandBuffer,
			vkIndirectBuf,
			offset,
			drawCount,
			sizeof(VkDrawIndexedIndirectCommand)
		);
	}

	void CommandBuffer::push_constants(const void*, uint32_t) {
		// Layout-Anbindung folgt bei Pipeline-Erstellung
	}
} // namespace axiom