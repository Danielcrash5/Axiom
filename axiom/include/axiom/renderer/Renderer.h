#pragma once
#include <vector>
#include "vulkan/VulkanContext.h"
#include "vulkan/VulkanSwapchain.h"
#include "vulkan/VulkanFrame.h"

class Renderer {
public:
	Renderer();
	~Renderer();

	void Init(GLFWwindow* window);
	void Shutdown();

	void BeginFrame();
	void EndFrame();

private:
	GLFWwindow* m_Window = nullptr;
	VulkanContext* m_Context = nullptr;
	VulkanSwapchain* m_Swapchain = nullptr;
	VkCommandPool m_CommandPool = VK_NULL_HANDLE;

	static const int MaxFramesInFlight = 3;
	std::vector<VulkanFrame> m_Frames;
	uint32_t m_CurrentFrame = 0;
	uint32_t m_CurrentImageIndex = 0;
	bool m_FrameStarted = false;

private:
	void CreateCommandPool();
	void CreateFrames();
	void CleanupFrames();
	void RecreateSwapchain();
};
