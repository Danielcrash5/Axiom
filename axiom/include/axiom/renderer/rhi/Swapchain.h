/**
 * @file axiom/include/axiom/renderer/rhi/Swapchain.h
 * @brief Declares the public interface for the swapchain.
 */

#pragma once
#include <cstdint>
#include "Handle.h"
#include "RHITypes.h"

namespace axiom::renderer::rhi {

/** @brief Stores the data for the swapchain desc. */
struct SwapchainDesc {
    /** @brief Stores the surface. */
    SurfaceHandle surface;
    /** @brief Stores the width. */
    uint32_t width = 0;
    /** @brief Stores the height. */
    uint32_t height = 0;
    /** @brief Stores the vsync. */
    bool vsync = true; // FIFO (an) vs. IMMEDIATE/MAILBOX (aus)
};

// texture ist ein stabiler TextureHandle, der beim Anlegen der Swapchain
// EINMAL pro Swapchain-Image erzeugt wird (siehe VulkanBackend::createSwapchain) -
// pro Frame aendert sich nur, WELCHES dieser Handles zurueckgegeben wird
// (imageIndex), nicht die Handles selbst. Kein Texture-Handle-Churn pro Frame.
/** @brief Stores the data for the acquired image. */
struct AcquiredImage {
    /** @brief Stores the texture. */
    TextureHandle texture;
    /** @brief Stores the image index. */
    uint32_t imageIndex = 0;
};

} // namespace axiom::renderer::rhi
