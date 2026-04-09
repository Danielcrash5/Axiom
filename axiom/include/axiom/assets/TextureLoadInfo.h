// TextureLoadInfo.h
#pragma once
#include "axiom/renderer/TextureOptions.h"

namespace axiom {

    struct TextureLoadInfo {
        bool sRGB = true;
        bool HDR = false;
        bool is16Bit = false;
        bool generateMipmaps = true;

        TextureWrap wrapS = TextureWrap::Repeat;
        TextureWrap wrapT = TextureWrap::Repeat;

        TextureFilter filterMin = TextureFilter::Linear;
        TextureFilter filterMag = TextureFilter::Linear;
    };

    struct TexturePresets {
        static TextureLoadInfo Albedo() {
            TextureLoadInfo i;
            i.sRGB = true;
            i.HDR = false;
            i.is16Bit = false;
            i.generateMipmaps = true;
            i.wrapS = TextureWrap::Repeat;
            i.wrapT = TextureWrap::Repeat;
            i.filterMin = TextureFilter::Linear;
            i.filterMag = TextureFilter::Linear;
            return i;
        }

        static TextureLoadInfo Sprite() {
            TextureLoadInfo i;
            i.sRGB = true;
            i.HDR = false;
            i.is16Bit = false;
            i.generateMipmaps = true;
            i.wrapS = TextureWrap::ClampToEdge;
            i.wrapT = TextureWrap::ClampToEdge;
            i.filterMin = TextureFilter::Nearest;
            i.filterMag = TextureFilter::Nearest;
            return i;
		}

        static TextureLoadInfo NormalMap() {
            TextureLoadInfo i;
            i.sRGB = false;
            i.HDR = false;
            i.is16Bit = false;
            i.generateMipmaps = true;
            i.wrapS = TextureWrap::Repeat;
            i.wrapT = TextureWrap::Repeat;
            i.filterMin = TextureFilter::Linear;
            i.filterMag = TextureFilter::Linear;
            return i;
        }

        static TextureLoadInfo UI() {
            TextureLoadInfo i;
            i.sRGB = true;
            i.HDR = false;
            i.is16Bit = false;
            i.generateMipmaps = false;
            i.wrapS = TextureWrap::ClampToEdge;
            i.wrapT = TextureWrap::ClampToEdge;
            i.filterMin = TextureFilter::Nearest;
            i.filterMag = TextureFilter::Nearest;
            return i;
        }
    };

} // namespace axiom