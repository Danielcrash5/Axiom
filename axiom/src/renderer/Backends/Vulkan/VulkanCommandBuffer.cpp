#include <Volk/volk.h>
#include <axiom/renderer/RendererRHI.h>
#include <stdexcept>

namespace axiom {
	struct CommandBufferImpl {
		VkCommandBuffer nativeCommandBuffer = VK_NULL_HANDLE;
		GraphicsDevice* deviceContext = nullptr;
		uint32_t imageIndex = 0;
	};

	CommandBuffer::CommandBuffer() {
		m_impl = new CommandBufferImpl();
	}

	CommandBuffer::~CommandBuffer() {
		delete m_impl;
	}

    void CommandBuffer::set_native_handle(void* cmdBuffer, GraphicsDevice* device, uint32_t swapchainImageIndex) {
        m_impl->nativeCommandBuffer = static_cast<VkCommandBuffer>(cmdBuffer);
        m_impl->deviceContext = device;
        m_impl->imageIndex = swapchainImageIndex;
    }

    // Interne Hilfsfunktion für Image Layout Transitions (Pipeline Barriers)
    void insert_image_barrier(VkCommandBuffer cmd, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, VkAccessFlags srcAccess, VkAccessFlags dstAccess, VkPipelineStageFlags srcStage, VkPipelineStageFlags dstStage) {
        VkImageMemoryBarrier barrier {
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .srcAccessMask = srcAccess,
            .dstAccessMask = dstAccess,
            .oldLayout = oldLayout,
            .newLayout = newLayout,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image = image,
            .subresourceRange = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1
            }
        };
        vkCmdPipelineBarrier(cmd, srcStage, dstStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
    }

    void CommandBuffer::begin_rendering(TextureHandle, float r, float g, float b, float a) {
        if (!m_impl || m_impl->nativeCommandBuffer == VK_NULL_HANDLE || !m_impl->deviceContext) return;

        VkImage swapchainImage = static_cast<VkImage>(m_impl->deviceContext->get_current_swapchain_image());
        VkImageView swapchainView = static_cast<VkImageView>(m_impl->deviceContext->get_current_swapchain_image_view());

        // 1. Swapchain Bild bereitmachen zum Beschreiben
        insert_image_barrier(m_impl->nativeCommandBuffer, swapchainImage,
                             VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                             0, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                             VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);

        // 2. Clear-Farbe definieren
        VkClearValue clearColor = { {{r, g, b, a}} };

        // 3. NEU: Dynamische Fenstergröße aus dem Device-Context abfragen
        uint32_t width = 0;
        uint32_t height = 0;
        m_impl->deviceContext->get_swapchain_extent(width, height);

        VkRenderingAttachmentInfo colorAttachmentInfo {
            .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
            .imageView = swapchainView,
            .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .clearValue = clearColor
        };

        // 4. Render Area nutzt jetzt die echten, dynamischen Fenster-Ausmaße!
        VkRenderingInfo renderingInfo {
            .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
            .renderArea = { {0, 0}, {width, height} },
            .layerCount = 1,
            .colorAttachmentCount = 1,
            .pColorAttachments = &colorAttachmentInfo
        };

        vkCmdBeginRendering(m_impl->nativeCommandBuffer, &renderingInfo);
    }

    void CommandBuffer::end_rendering() {
        if (!m_impl || m_impl->nativeCommandBuffer == VK_NULL_HANDLE || !m_impl->deviceContext) return;

        // Rendering beenden
        vkCmdEndRendering(m_impl->nativeCommandBuffer);

        VkImage swapchainImage = static_cast<VkImage>(m_impl->deviceContext->get_current_swapchain_image());

        // 4. Bild wieder zurücktransformieren, damit die Windows-Oberfläche (Present) es anzeigen darf
        insert_image_barrier(m_impl->nativeCommandBuffer, swapchainImage,
                             VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                             VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, 0,
                             VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
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