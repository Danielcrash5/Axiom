#pragma once
#include "TextureEnums.h"
#include <cstdint>

namespace axiom {

    enum class TextureType {
        Texture2D,
        CubeMap
    };

    enum class TextureFormat {
        RGBA8,
        RGB8,
        R8
    };

    struct TextureSpec {
        TextureType Type = TextureType::Texture2D;
        TextureFormat Format = TextureFormat::RGBA8;

        uint32_t Width = 1;
        uint32_t Height = 1;

        bool GenerateMipmaps = false;
        bool FlipOnLoad = true;

        bool AnisotropicFiltering = false;

        // Sampling
        TextureWrap WrapS = TextureWrap::Repeat;
        TextureWrap WrapT = TextureWrap::Repeat;
        TextureFilter MinFilter = TextureFilter::Linear;
        TextureFilter MagFilter = TextureFilter::Linear;

        // Pipeline
        bool UseBindless = true;
    };

}