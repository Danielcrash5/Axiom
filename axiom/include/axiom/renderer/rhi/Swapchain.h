#pragma once
#include <cstdint>
#include "Handle.h"
#include "RHITypes.h"

namespace axiom::renderer::rhi {

struct SwapchainDesc {
    SurfaceHandle surface;
    uint32_t width = 0;
    uint32_t height = 0;
    bool vsync = true; // FIFO (an) vs. IMMEDIATE/MAILBOX (aus)
};

// texture ist ein stabiler TextureHandle, der beim Anlegen der Swapchain
// EINMAL pro Swapchain-Image erzeugt wird (siehe VulkanBackend::createSwapchain) -
// pro Frame aendert sich nur, WELCHES dieser Handles zurueckgegeben wird
// (imageIndex), nicht die Handles selbst. Kein Texture-Handle-Churn pro Frame.
struct AcquiredImage {
    TextureHandle texture;
    uint32_t imageIndex = 0;
};

} // namespace axiom::renderer::rhi
