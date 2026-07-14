#pragma once
#include <axiom/renderer/rhi/RHITypes.h>
#include <cstdint>
#include <string_view>

namespace axiom::renderer::rendergraph {

    enum class ResourceType { Texture, Buffer };

    struct TextureResourceDesc {
        uint32_t width = 0;
        uint32_t height = 0;
        rhi::TextureFormat format = rhi::TextureFormat::RGBA8Unorm;
        rhi::TextureUsage usage = rhi::TextureUsage::None;
        std::string_view debugName;
    };

    // Für Dependency-Auflösung: was macht der Pass mit der Resource.
    enum class AccessType { Read, Write };

} // namespace axiom::renderer::rendergraph