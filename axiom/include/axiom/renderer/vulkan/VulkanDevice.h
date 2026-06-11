#pragma once

#include <Volk/volk.h>

#include "VulkanPhysicalDevice.h"

namespace axiom {
    class VulkanDevice {
    public:
        VulkanDevice() = default;
        ~VulkanDevice() = default;

        void Create(
            const VulkanPhysicalDevice& physicalDevice);

        void Destroy();

        VkDevice GetHandle() const {
            return m_Device;
        }

        VkQueue GetGraphicsQueue() const {
            return m_GraphicsQueue;
        }

        VkQueue GetPresentQueue() const {
            return m_PresentQueue;
        }

    private:
        VkDevice m_Device = VK_NULL_HANDLE;

        VkQueue m_GraphicsQueue = VK_NULL_HANDLE;
        VkQueue m_PresentQueue = VK_NULL_HANDLE;
    };
}