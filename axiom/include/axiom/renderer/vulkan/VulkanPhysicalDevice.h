#pragma once

#include <Volk/volk.h>

namespace axiom {
    struct QueueFamilyIndices {
        uint32_t GraphicsFamily = UINT32_MAX;
        uint32_t PresentFamily = UINT32_MAX;

        bool IsComplete() const {
            return GraphicsFamily != UINT32_MAX &&
                PresentFamily != UINT32_MAX;
        }
    };

    class VulkanPhysicalDevice {
    public:
        VulkanPhysicalDevice() = default;
        ~VulkanPhysicalDevice() = default;

        void Pick(
            VkInstance instance,
            VkSurfaceKHR surface);

        VkPhysicalDevice GetHandle() const {
            return m_PhysicalDevice;
        }

        const VkPhysicalDeviceProperties& GetProperties() const {
            return m_Properties;
        }

        const VkPhysicalDeviceFeatures& GetFeatures() const {
            return m_Features;
        }

        const QueueFamilyIndices& GetQueueFamilies() const {
            return m_QueueFamilies;
        }

    private:
        QueueFamilyIndices FindQueueFamilies(
            VkPhysicalDevice device,
            VkSurfaceKHR surface);

    private:
        VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;

        VkPhysicalDeviceProperties m_Properties {};
        VkPhysicalDeviceFeatures   m_Features {};

        QueueFamilyIndices m_QueueFamilies;
    };
}