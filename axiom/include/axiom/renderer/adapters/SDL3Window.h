#pragma once
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <vulkan/vulkan.h>
#include <axiom/renderer/Renderer.h>

namespace axiom::renderer::adapters {

inline WindowSurfaceDesc makeSDL3WindowSurfaceDesc(SDL_Window* window) {
    WindowSurfaceDesc desc;

    uint32_t extCount = 0;
    auto extNames = SDL_Vulkan_GetInstanceExtensions(&extCount);
    desc.requiredInstanceExtensions.assign(extNames, extNames + extCount);

    desc.createSurface = [window](void* nativeInstance) -> rhi::RHIResult<void*> {
        VkSurfaceKHR surface = VK_NULL_HANDLE;
        if (!SDL_Vulkan_CreateSurface(window, static_cast<VkInstance>(nativeInstance), nullptr, &surface)) {
            return std::unexpected(rhi::RHIError::Unknown);
        }
        return reinterpret_cast<void*>(surface);
    };

    return desc;
}

} // namespace axiom::renderer::adapters
