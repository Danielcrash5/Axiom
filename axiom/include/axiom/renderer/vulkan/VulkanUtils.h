#pragma once
#include <vulkan/vulkan.h>
#include <stdexcept>
#include <string>

inline void VKCheck(VkResult result, const std::string& msg)
{
   if (result != VK_SUCCESS)
        throw std::runtime_error(msg);
}