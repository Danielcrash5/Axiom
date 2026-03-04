#include "axiom/renderer/Renderer.h"
#include "axiom/renderer/vulkan/VulkanUtils.h"
#include "axiom/core/Logger.h"
#include <stdexcept>

Renderer::Renderer() {}

Renderer::~Renderer() {
	Shutdown();
}

void Renderer::Init(GLFWwindow* window) {
	m_Window = window;
	m_Context = new VulkanContext();
	m_Context->Init(m_Window);

	if (m_Context->GetDevice() == VK_NULL_HANDLE)
		AXIOM_ERROR("Vulkan logical device is null!");
	if (m_Context->GetPhysicalDevice() == VK_NULL_HANDLE)
		AXIOM_ERROR("Vulkan physical device is null!");

	m_Swapchain = new VulkanSwapchain(m_Context);
	m_Swapchain->Init();

	CreateCommandPool();
	CreateFrames();

	AXIOM_INFO("Renderer initialized successfully.");
}

void Renderer::Shutdown() {
	if (!m_Context) return;

	vkDeviceWaitIdle(m_Context->GetDevice());

	CleanupFrames();

	if (m_CommandPool != VK_NULL_HANDLE) {
		vkDestroyCommandPool(m_Context->GetDevice(), m_CommandPool, nullptr);
		m_CommandPool = VK_NULL_HANDLE;
	}

	if (m_Swapchain) { delete m_Swapchain; m_Swapchain = nullptr; }
	if (m_Context) { delete m_Context; m_Context = nullptr; }
}

// ----- Command pool -----
void Renderer::CreateCommandPool() {
	VkCommandPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	poolInfo.queueFamilyIndex = m_Context->GetGraphicsQueueFamilyIndex();

	VKCheck(vkCreateCommandPool(m_Context->GetDevice(), &poolInfo, nullptr, &m_CommandPool),
			"Failed to create command pool!");
}

// ----- Frame handling -----
void Renderer::CreateFrames() {
	m_Frames.resize(MaxFramesInFlight);

	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = m_CommandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = 1;

	for (auto& frame : m_Frames) {
		VKCheck(vkAllocateCommandBuffers(m_Context->GetDevice(), &allocInfo, &frame.CommandBuffer),
				"Failed to allocate command buffer!");

		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VKCheck(vkCreateSemaphore(m_Context->GetDevice(), &semaphoreInfo, nullptr, &frame.ImageAvailableSemaphore),
				"Failed to create ImageAvailableSemaphore!");
		VKCheck(vkCreateSemaphore(m_Context->GetDevice(), &semaphoreInfo, nullptr, &frame.RenderFinishedSemaphore),
				"Failed to create RenderFinishedSemaphore!");

		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		VKCheck(vkCreateFence(m_Context->GetDevice(), &fenceInfo, nullptr, &frame.InFlightFence),
				"Failed to create InFlightFence!");
	}
}

void Renderer::CleanupFrames() {
	for (auto& frame : m_Frames) {
		if (frame.ImageAvailableSemaphore != VK_NULL_HANDLE)
			vkDestroySemaphore(m_Context->GetDevice(), frame.ImageAvailableSemaphore, nullptr);
		if (frame.RenderFinishedSemaphore != VK_NULL_HANDLE)
			vkDestroySemaphore(m_Context->GetDevice(), frame.RenderFinishedSemaphore, nullptr);
		if (frame.InFlightFence != VK_NULL_HANDLE)
			vkDestroyFence(m_Context->GetDevice(), frame.InFlightFence, nullptr);
	}
	m_Frames.clear();
}

// ----- Frame rendering -----
void Renderer::BeginFrame() {
	auto& frame = m_Frames[m_CurrentFrame];

	vkWaitForFences(m_Context->GetDevice(), 1, &frame.InFlightFence, VK_TRUE, UINT64_MAX);

	VkResult result = vkAcquireNextImageKHR(
		m_Context->GetDevice(),
		m_Swapchain->GetSwapchain(),
		UINT64_MAX,
		frame.ImageAvailableSemaphore,
		VK_NULL_HANDLE,
		&m_CurrentImageIndex);

	if (result == VK_ERROR_OUT_OF_DATE_KHR) {
		RecreateSwapchain();
		m_FrameStarted = false;
		return;
	}
	if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
		throw std::runtime_error("Failed to acquire swapchain image!");

	m_FrameStarted = true;
	vkResetFences(m_Context->GetDevice(), 1, &frame.InFlightFence);
	vkResetCommandBuffer(frame.CommandBuffer, 0);

	// Begin command buffer
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	VKCheck(vkBeginCommandBuffer(frame.CommandBuffer, &beginInfo),
			"Failed to begin command buffer!");

	// Transition image: UNDEFINED -> COLOR_ATTACHMENT_OPTIMAL
	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = m_Swapchain->GetImages()[m_CurrentImageIndex];
	barrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
	barrier.srcAccessMask = 0;
	barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	vkCmdPipelineBarrier(frame.CommandBuffer,
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		0, 0, nullptr, 0, nullptr, 1, &barrier);

	// Begin dynamic rendering (clear color)
	VkRenderingAttachmentInfo colorAttachment{};
	colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
	colorAttachment.imageView = m_Swapchain->GetImageViews()[m_CurrentImageIndex];
	colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.clearValue.color = {{0.1f, 0.1f, 0.1f, 1.0f}};

	VkRenderingInfo renderingInfo{};
	renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
	renderingInfo.renderArea = {{0, 0}, m_Swapchain->GetExtent()};
	renderingInfo.layerCount = 1;
	renderingInfo.colorAttachmentCount = 1;
	renderingInfo.pColorAttachments = &colorAttachment;

	vkCmdBeginRendering(frame.CommandBuffer, &renderingInfo);
}

void Renderer::EndFrame() {
	if (!m_FrameStarted) return;

	auto& frame = m_Frames[m_CurrentFrame];

	vkCmdEndRendering(frame.CommandBuffer);

	// Transition image: COLOR_ATTACHMENT_OPTIMAL -> PRESENT_SRC_KHR
	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = m_Swapchain->GetImages()[m_CurrentImageIndex];
	barrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
	barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	barrier.dstAccessMask = 0;

	vkCmdPipelineBarrier(frame.CommandBuffer,
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
		0, 0, nullptr, 0, nullptr, 1, &barrier);

	VKCheck(vkEndCommandBuffer(frame.CommandBuffer),
			"Failed to end command buffer!");

	// Submit
	VkSemaphore waitSemaphores[] = { frame.ImageAvailableSemaphore };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	VkSemaphore signalSemaphores[] = { frame.RenderFinishedSemaphore };

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &frame.CommandBuffer;
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	VKCheck(vkQueueSubmit(m_Context->GetGraphicsQueue(), 1, &submitInfo, frame.InFlightFence),
			"Failed to submit draw command buffer!");

	// Present
	VkSwapchainKHR swapchains[] = { m_Swapchain->GetSwapchain() };

	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapchains;
	presentInfo.pImageIndices = &m_CurrentImageIndex;

	VkResult result = vkQueuePresentKHR(m_Context->GetPresentQueue(), &presentInfo);

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
		RecreateSwapchain();
	} else if (result != VK_SUCCESS) {
		throw std::runtime_error("Failed to present swapchain image!");
	}

	m_CurrentFrame = (m_CurrentFrame + 1) % MaxFramesInFlight;
}

void Renderer::RecreateSwapchain() {
	int width = 0, height = 0;
	glfwGetFramebufferSize(m_Window, &width, &height);
	while (width == 0 || height == 0) {
		glfwGetFramebufferSize(m_Window, &width, &height);
		glfwWaitEvents();
	}

	vkDeviceWaitIdle(m_Context->GetDevice());
	m_Swapchain->Recreate();
}
