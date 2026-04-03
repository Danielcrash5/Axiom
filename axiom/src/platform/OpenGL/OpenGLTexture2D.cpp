#include "axiom/platform/OpenGL/OpenGLTexture2D.h"
#include "axiom/core/Logger.h"
#include <glad/glad.h>
#include <stb_image.h>


namespace axiom {
	GLenum toGL(TextureFilter filter) {
		switch (filter) {
		case axiom::TextureFilter::Nearest:
			return GL_NEAREST;
			break;
		case axiom::TextureFilter::Linear:
			return GL_LINEAR;
			break;
		default:
			break;
		}
	}

	GLenum toGL(TextureFormat format) {
		switch (format) {
		case TextureFormat::None:               return GL_NONE;

			// 8-Bit
		case TextureFormat::R8:                 return GL_R8;
		case TextureFormat::RG8:                return GL_RG8;
		case TextureFormat::RGB8:               return GL_RGB8;
		case TextureFormat::RGBA8:              return GL_RGBA8;

			// sRGB
		case TextureFormat::SRGB8:              return GL_SRGB8;
		case TextureFormat::SRGB8_ALPHA8:       return GL_SRGB8_ALPHA8;

			// 16-Bit
		case TextureFormat::R16:                return GL_R16;
		case TextureFormat::RG16:               return GL_RG16;
		case TextureFormat::RGB16:              return GL_RGB16;
		case TextureFormat::RGBA16:             return GL_RGBA16;

			// 16-Bit Float
		case TextureFormat::R16F:               return GL_R16F;
		case TextureFormat::RG16F:              return GL_RG16F;
		case TextureFormat::RGB16F:             return GL_RGB16F;
		case TextureFormat::RGBA16F:            return GL_RGBA16F;

			// 32-Bit Float
		case TextureFormat::R32F:               return GL_R32F;
		case TextureFormat::RG32F:              return GL_RG32F;
		case TextureFormat::RGB32F:             return GL_RGB32F;
		case TextureFormat::RGBA32F:            return GL_RGBA32F;

			// Depth / Stencil
		case TextureFormat::Depth16:            return GL_DEPTH_COMPONENT16;
		case TextureFormat::Depth24:            return GL_DEPTH_COMPONENT24;
		case TextureFormat::Depth32:            return GL_DEPTH_COMPONENT32;
		case TextureFormat::Depth32F:           return GL_DEPTH_COMPONENT32F;
		case TextureFormat::Depth24Stencil8:    return GL_DEPTH24_STENCIL8;
		case TextureFormat::Depth32FStencil8:   return GL_DEPTH32F_STENCIL8;

		default:
			break;
		}
	}

	GLenum toGL(TextureWrap wrap) {
		switch (wrap) {
		case axiom::TextureWrap::Repeat:
			return GL_REPEAT;
			break;
		case axiom::TextureWrap::ClampToEdge:
			return GL_CLAMP_TO_EDGE;
			break;
		case axiom::TextureWrap::ClampToBorder:
			return GL_CLAMP_TO_BORDER;
			break;
		case axiom::TextureWrap::MirroredRepeat:
			return GL_CLAMP_TO_BORDER;
			break;
		default:
			break;
		}
	}


	OpenGLTexture2D::OpenGLTexture2D(int width, int height, bool generateMipmaps,
									 TextureWrap wrapS, TextureWrap wrapT, TextureFilter filterMin, TextureFilter filterMag)
		: m_Width(width), m_Height(height), m_GenerateMipmaps(generateMipmaps),
		m_WrapS(wrapS), m_WrapT(wrapT), m_FilterMin(filterMin), m_FilterMag(filterMag) {
		glGenTextures(1, &m_RendererID);
		glBindTexture(GL_TEXTURE_2D, m_RendererID);

		// Default Format RGBA8
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

		SetWrap(wrapS, wrapT);
		SetFilter(filterMin, filterMag);

		if (generateMipmaps)
			glGenerateMipmap(GL_TEXTURE_2D);

		glBindTexture(GL_TEXTURE_2D, 0);

		// Bindless
		if (GLAD_GL_ARB_bindless_texture) {
			m_BindlessHandle = glGetTextureHandleARB(m_RendererID);
			glMakeTextureHandleResidentARB(m_BindlessHandle);
		}
	}

	OpenGLTexture2D::~OpenGLTexture2D() {
		if (m_BindlessHandle)
			glMakeTextureHandleNonResidentARB(m_BindlessHandle);
		glDeleteTextures(1, &m_RendererID);
	}

	void OpenGLTexture2D::Bind(int slot) const {
		glBindTextureUnit(slot, m_RendererID);
	}

	uint64_t OpenGLTexture2D::GetBindlessHandle() const {
		return m_BindlessHandle;
	}

	bool OpenGLTexture2D::SupportsBindless() const {
		return m_BindlessHandle != 0;
	}

	void OpenGLTexture2D::SetData(const void* Data, uint32_t width, uint32_t height, TextureFormat format) {
		glBindTexture(GL_TEXTURE_2D, m_RendererID);
		GLenum glFormat = toGL(format);
		glTexImage2D(GL_TEXTURE_2D, 0, glFormat, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, Data);

		if (m_GenerateMipmaps)
			glGenerateMipmap(GL_TEXTURE_2D);

		glBindTexture(GL_TEXTURE_2D, 0);
	}

	void OpenGLTexture2D::LoadFromFile(const std::string& path, bool sRGB, bool HDR, bool is16Bit) {
		int width, height, channels;
		void* data = nullptr;

		if (HDR) {
			float* floatData = stbi_loadf(path.c_str(), &width, &height, &channels, 0);
			data = floatData;
			m_TextureFormat = TextureFormat::RGBA16F; // oder RGBA32F bei Bedarf
		}
		else {
			stbi_set_flip_vertically_on_load(1);
			unsigned char* imgData = stbi_load(path.c_str(), &width, &height, &channels, 0);
			data = imgData;

			if (is16Bit)
				m_TextureFormat = TextureFormat::RGBA16;
			else
				m_TextureFormat = sRGB ? TextureFormat::SRGB8_ALPHA8 : TextureFormat::RGBA8;
		}

		if (!data) {
			AXIOM_ERROR("[Texture2D] Failed to load texture: {}", path);
			return;
		}

		SetData(data, width, height, m_TextureFormat);

		if (!HDR)
			stbi_image_free(data);
		else
			stbi_image_free(static_cast<float*>(data));

		m_Width = width;
		m_Height = height;
	}

	void OpenGLTexture2D::SetWrap(TextureWrap wrapS, TextureWrap wrapT) {
		glBindTexture(GL_TEXTURE_2D, m_RendererID);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, toGL(wrapS));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, toGL(wrapT));
		glBindTexture(GL_TEXTURE_2D, 0);

		m_WrapS = wrapS;
		m_WrapT = wrapT;
	}

	void OpenGLTexture2D::SetFilter(TextureFilter filterMin, TextureFilter filterMag) {
		glBindTexture(GL_TEXTURE_2D, m_RendererID);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, toGL(filterMin));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, toGL(filterMag));
		glBindTexture(GL_TEXTURE_2D, 0);

		m_FilterMin = filterMin;
		m_FilterMag = filterMag;
	}

	void OpenGLTexture2D::SetFormat(TextureFormat textureFormat) {
		m_TextureFormat = textureFormat;
		glBindTexture(GL_TEXTURE_2D, m_RendererID);
		glTexImage2D(GL_TEXTURE_2D, 0, toGL(textureFormat), m_Width, m_Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	void OpenGLTexture2D::GenerateMipmaps() {
		glBindTexture(GL_TEXTURE_2D, m_RendererID);
		glGenerateMipmap(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

} // namespace axiom