#pragma once
#include <vulkan/vulkan.h>

struct VulkanFrame
{
    VkCommandBuffer CommandBuffer = VK_NULL_HANDLE;
    VkSemaphore ImageAvailableSemaphore = VK_NULL_HANDLE;
    VkSemaphore RenderFinishedSemaphore = VK_NULL_HANDLE;
    VkFence InFlightFence = VK_NULL_HANDLE;
};