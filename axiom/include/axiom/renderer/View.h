/**
 * @file axiom/include/axiom/renderer/View.h
 * @brief Declares the public interface for the view.
 */

#pragma once

#include <axiom/renderer/RenderLayerRegistry.h>
#include <axiom/renderer/rhi/Handle.h>
#include <glm/glm.hpp>
#include <cstdint>

namespace axiom::renderer {

/** @brief Stores the data for the viewport. */
struct Viewport {
    /** @brief Stores the x. */
    uint32_t x = 0;
    /** @brief Stores the y. */
    uint32_t y = 0;
    /** @brief Stores the width. */
    uint32_t width = 0;
    /** @brief Stores the height. */
    uint32_t height = 0;
};

/**
 * @brief Defines the available render target kind values.
 * @tparam RenderTargetKind Template type used by this type.
 */
enum class RenderTargetKind {
    /** @brief Represents the swapchain value. */
    swapchain,
    /** @brief Represents the texture value. */
    texture,
};

/** @brief Stores the data for the render target. */
struct RenderTarget {
    /** @brief Stores the kind. */
    RenderTargetKind kind = RenderTargetKind::swapchain;
    /** @brief Stores the texture. */
    rhi::TextureHandle texture;
    /** @brief Stores the surface. */
    rhi::SurfaceHandle surface;
};

/** @brief Describes a render target, viewport, and related view settings. */
struct View {
    /** @brief Stores the view matrix. */
    glm::mat4 viewMatrix{1.0f};
    /** @brief Stores the projection matrix. */
    glm::mat4 projectionMatrix{1.0f};
    /** @brief Stores the visible layers. */
    RenderLayerMask visibleLayers = ~RenderLayerMask{0};
    /** @brief Stores the target. */
    RenderTarget target;
    /** @brief Stores the viewport. */
    Viewport viewport;
    /** @brief Stores the priority. */
    int priority = 0;
};

/**
 * @brief Provides the view ID type alias.
 */
using ViewID = uint32_t;
/** @brief Defines the k invalid view ID constant. */
inline constexpr ViewID kInvalidViewID = 0;

} // namespace axiom::renderer
