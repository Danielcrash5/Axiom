#include "axiom/renderer/Texture2D.h"
#include "axiom/renderer/RendererAPI.h"
#include "axiom/platform/OpenGL/OpenGLTexture2D.h"

namespace axiom {
	std::shared_ptr<Texture2D> Texture2D::Create(int width, int height, bool generateMipmaps, TextureWrap wraps, TextureWrap wrapT, TextureFilter filterMin, TextureFilter filterMag) {
		if (RendererAPI::GetAPI() == RendererAPIType::OpenGL) {
			return std::make_shared<OpenGLTexture2D>(width, height, generateMipmaps, wraps, wrapT, filterMin, filterMag);
		}
	}

	std::shared_ptr<Texture2D> Texture2D::Create(const std::string& path, bool sRGB, bool HDR, bool is16Bit, bool generateMipmaps, TextureWrap wraps, TextureWrap wrapT, TextureFilter filterMin, TextureFilter filterMag) {
		if (RendererAPI::GetAPI() == RendererAPIType::OpenGL) {
			auto texture = std::make_shared<OpenGLTexture2D>(32, 32, generateMipmaps, wraps, wrapT, filterMin, filterMag);
			texture->LoadFromFile(path, sRGB, HDR, is16Bit);
			return texture;
		}
	}
}