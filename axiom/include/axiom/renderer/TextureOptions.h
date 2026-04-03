#pragma once

namespace axiom {
    enum class TextureFormat {
        None = 0,

        // 8-Bit
        R8, RG8, RGB8, RGBA8,

        //SRGB
        SRGB8, SRGB8_ALPHA8,

        // 16-Bit
        R16, RG16, RGB16, RGBA16,

        // 16-Bit Float
        R16F, RG16F, RGB16F, RGBA16F,

        // 32-Bit Float
        R32F, RG32F, RGB32F, RGBA32F,

        // Depth / Stencil
        Depth16, Depth24, Depth32, Depth32F, Depth24Stencil8, Depth32FStencil8
    };

    enum class TextureFilter {
        Nearest,
        Linear
    };

    enum class TextureWrap {
        Repeat,
        ClampToEdge,
        ClampToBorder,
        MirroredRepeat
    };
}