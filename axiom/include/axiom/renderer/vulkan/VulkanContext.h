#pragma once
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <optional>
#include <vector>
#include <string>

struct QueueFamilyIndices {
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;

	bool IsComplete() const { return graphicsFamily.has_value() && presentFamily.has_value(); }
};

class VulkanContext {
public:
	VulkanContext(GLFWwindow* window);
	~VulkanContext();

	void Init();
	void Shutdown();

	void PickQueueFamilies(GLFWwindow* window);

	VkInstance GetInstance() const { return m_Instance; }
	VkDevice GetDevice() const { return m_Device; }
	VkPhysicalDevice GetPhysicalDevice() const { return m_PhysicalDevice; }
	uint32_t GetGraphicsQueueFamilyIndex() {
		return m_Indices.graphicsFamily.value();
	}

private:
	GLFWwindow* m_Window = nullptr;

	VkInstance m_Instance = VK_NULL_HANDLE;
	VkDebugUtilsMessengerEXT m_DebugMessenger = VK_NULL_HANDLE;
	VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
	VkDevice m_Device = VK_NULL_HANDLE;

	QueueFamilyIndices m_Indices;

	bool m_EnableValidationLayers = true;

	const std::vector<const char*> m_ValidationLayers = {
		"VK_LAYER_KHRONOS_validation"
	};

	void CreateInstance();
	void SetupDebugMessenger();
	void PickPhysicalDevice();
	void CreateLogicalDevice();
	void DestroyDebugMessenger();

	// Helper
	bool CheckValidationLayerSupport();
	std::vector<const char*> GetRequiredExtensions();
	static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT       messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT              messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData);
};