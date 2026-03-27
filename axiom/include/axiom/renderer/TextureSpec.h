#pragma once
#include "TextureEnums.h"

namespace axiom {

    struct TextureSpec {
        TextureWrap WrapS = TextureWrap::Repeat;
        TextureWrap WrapT = TextureWrap::Repeat;

        TextureFilter MinFilter = TextureFilter::Linear;
        TextureFilter MagFilter = TextureFilter::Linear;

        bool GenerateMipmaps = false;
        bool FlipOnLoad = true;
    };

}