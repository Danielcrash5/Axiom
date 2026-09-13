#pragma once

#include <axiom/renderer/RenderLayerRegistry.h>
#include <axiom/renderer/rhi/Handle.h>
#include <glm/glm.hpp>
#include <cstdint>

namespace axiom::renderer {

struct Viewport {
    uint32_t x = 0;
    uint32_t y = 0;
    uint32_t width = 0;
    uint32_t height = 0;
};

enum class RenderTargetKind {
    Swapchain,
    Texture,
};

struct RenderTarget {
    RenderTargetKind kind = RenderTargetKind::Swapchain;
    rhi::TextureHandle texture;
    rhi::SurfaceHandle surface;
};

struct View {
    glm::mat4 viewMatrix{1.0f};
    glm::mat4 projectionMatrix{1.0f};
    RenderLayerMask visibleLayers = ~RenderLayerMask{0};
    RenderTarget target;
    Viewport viewport;
    int priority = 0;
};

using ViewID = uint32_t;
inline constexpr ViewID kInvalidViewID = 0;

} // namespace axiom::renderer
