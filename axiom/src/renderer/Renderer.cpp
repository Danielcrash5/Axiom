#include "axiom/renderer/Renderer.h"
#include "axiom/renderer/vulkan/VulkanUtils.h"
#include "axiom/core/Logger.h"

Renderer::Renderer() {}

Renderer::~Renderer() {
	Shutdown();
}

void Renderer::Init(GLFWwindow* window) {
	m_Window = window;
	m_Context = new VulkanContext();
	m_Context->Init(m_Window);

	if (m_Context->GetDevice() == VK_NULL_HANDLE)
		AXIOM_ERROR("Vulkan logical device ist null!");
	if (m_Context->GetPhysicalDevice() == VK_NULL_HANDLE)
		AXIOM_ERROR("Vulkan physical device ist null!");

	m_Swapchain = new VulkanSwapchain(m_Context);
	m_Swapchain->Init();

	CreateFrames();

	AXIOM_INFO("Renderer initialized successfully.");
}

void Renderer::Shutdown() {
	CleanupFrames();

	if (m_Swapchain) { m_Swapchain->Cleanup(); delete m_Swapchain; m_Swapchain = nullptr; }
	if (m_Context) { m_Context->Cleanup(); delete m_Context; m_Context = nullptr; }
}

// ----- Frame handling -----
void Renderer::CreateFrames() {
	m_Frames.resize(MaxFramesInFlight);

	for (auto& frame : m_Frames) {
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
}

void Renderer::BeginFrame() {
	auto& frame = m_Frames[m_CurrentFrame];

	vkWaitForFences(m_Context->GetDevice(), 1, &frame.InFlightFence, VK_TRUE, UINT64_MAX);
	vkResetFences(m_Context->GetDevice(), 1, &frame.InFlightFence);

	// TODO: CommandBuffer allocation and begin
}

void Renderer::EndFrame() {
	auto& frame = m_Frames[m_CurrentFrame];

	// TODO: Submit CommandBuffer + Present

	m_CurrentFrame = (m_CurrentFrame + 1) % MaxFramesInFlight;
}