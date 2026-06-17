#pragma once

#include <cstdint>

namespace axiom {
    enum class TextureDimension {
        Texture1D,
        Texture2D,
        Texture3D,
        TextureCube,
        Texture2DArray,
        TextureCubeArray
    };

    enum class TextureFormat {
        RGBA8_UNORM,
        RGBA8_SRGB,

        RGBA16_FLOAT,
        RGBA32_FLOAT,

        D24S8,
        D32_FLOAT
    };

    enum class TextureUsage : uint32_t {
        None = 0,
        Sampled = 1 << 0,
        RenderTarget = 1 << 1,
        DepthStencil = 1 << 2,
        Storage = 1 << 3,
        TransferSrc = 1 << 4,
        TransferDst = 1 << 5
    };

    struct TextureDesc {
        TextureDimension dimension =
            TextureDimension::Texture2D;

        TextureFormat format =
            TextureFormat::RGBA8_UNORM;

        uint32_t width = 1;
        uint32_t height = 1;
        uint32_t depth = 1;

        uint32_t mip_levels = 1;
        uint32_t layers = 1;

        TextureUsage usage =
            TextureUsage::Sampled;
    };
}