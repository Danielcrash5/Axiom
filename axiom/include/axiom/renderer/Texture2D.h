#pragma once
#include <string>
#include <memory>
#include "TextureOptions.h"

namespace axiom {
    

	class Texture2D {
    public:
		virtual ~Texture2D() = 0;

        virtual void Bind(int slot) const = 0;
        virtual uint64_t GetBindlessHandle() const = 0;
        virtual bool SupportsBindless() const = 0;

        virtual void SetData(const void* Data, uint32_t width, uint32_t height, TextureFormat format) = 0;
        virtual void LoadFromFile(const std::string& path, bool sRGB, bool HDR, bool is16Bit) = 0;

        virtual void SetWrap(TextureWrap wrapS, TextureWrap wrapT) = 0;
        virtual void SetFilter(TextureFilter filterMin, TextureFilter filterMag) = 0;
        virtual void SetFormat(TextureFormat textureFormat) = 0;
        virtual void GenerateMipmaps() = 0;
        
        static std::shared_ptr<Texture2D> Create(int width, int height, bool generateMipmaps, TextureWrap wraps, TextureWrap wrapT, TextureFilter filterMin, TextureFilter filterMag);
	};
}