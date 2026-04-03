#include "axiom/renderer/Texture2D.h"
#include "axiom/renderer/RendererAPI.h"
#include "axiom/platform/OpenGL/OpenGLTexture2D.h"

namespace axiom {
	std::shared_ptr<Texture2D> Texture2D::Create(int width, int height, bool generateMipmaps, TextureWrap wraps, TextureWrap wrapT, TextureFilter filterMin, TextureFilter filterMag) {
		if (RendererAPI::GetAPI() == RendererAPIType::OpenGL) {
			return std::make_shared<OpenGLTexture2D>(width, height, generateMipmaps, wraps, wrapT, filterMin, filterMag);
		}
	}
}