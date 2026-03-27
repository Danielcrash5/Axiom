#include "axiom/renderer/Texture.h"
#include "axiom/core/Logger.h"
#include <glad/glad.h>
#include <stb_image.h>

namespace axiom {

	bool Texture::s_BindlessSupported = false;

	// ===================== Mapping =====================

	static GLenum ToGLWrap(TextureWrap wrap) {
		switch (wrap) {
		case TextureWrap::Repeat: return GL_REPEAT;
		case TextureWrap::Clamp:  return GL_CLAMP_TO_EDGE;
		case TextureWrap::Mirror: return GL_MIRRORED_REPEAT;
		}
		return GL_REPEAT;
	}

	static GLenum ToGLFilter(TextureFilter filter, bool mipmaps = false) {
		if (mipmaps) {
			return (filter == TextureFilter::Linear)
				? GL_LINEAR_MIPMAP_LINEAR
				: GL_NEAREST_MIPMAP_NEAREST;
		}
		return (filter == TextureFilter::Linear) ? GL_LINEAR : GL_NEAREST;
	}

	static GLenum ChannelsToFormat(int channels) {
		switch (channels) {
		case 1: return GL_RED;
		case 3: return GL_RGB;
		case 4: return GL_RGBA;
		}
		return GL_RGBA;
	}

	// ===================== Konstruktor =====================

	Texture::Texture(const std::string& path, const TextureSpec& spec)
		: m_Spec(spec) {
		LoadFromFile(path);
		ApplyParameters();
		MakeBindless();
	}

	Texture::Texture(uint32_t width, uint32_t height, const TextureSpec& spec)
		: m_Width(width), m_Height(height), m_Spec(spec) {
		CreateEmpty();
		ApplyParameters();
		MakeBindless();
	}

	Texture::~Texture() {
		if (m_IsBindless)
			glMakeTextureHandleNonResidentARB(m_Handle);

		if (m_RendererID)
			glDeleteTextures(1, &m_RendererID);
	}

	// ===================== Creation =====================

	void Texture::CreateEmpty() {
		glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
		glTextureStorage2D(m_RendererID, 1, GL_RGBA8, m_Width, m_Height);
	}

	void Texture::LoadFromFile(const std::string& path) {
		stbi_set_flip_vertically_on_load(m_Spec.FlipOnLoad);

		int channels;
		unsigned char* data = stbi_load(path.c_str(), (int*)&m_Width, (int*)&m_Height, &channels, 0);

		AXIOM_ASSERT(data, "Failed to load texture {}", path);

		GLenum format = ChannelsToFormat(channels);

		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);

		GLenum internalFormat =
			(format == GL_RGB) ? GL_RGB8 :
			(format == GL_RED) ? GL_R8 : GL_RGBA8;

		glTextureStorage2D(m_RendererID, 1, internalFormat, m_Width, m_Height);

		glTextureSubImage2D(
			m_RendererID,
			0,
			0, 0,
			m_Width,
			m_Height,
			format,
			GL_UNSIGNED_BYTE,
			data
		);

		if (m_Spec.GenerateMipmaps)
			glGenerateTextureMipmap(m_RendererID);

		MakeBindless();

		stbi_image_free(data);
	}

	void Texture::UploadData(void* data, uint32_t width, uint32_t height, GLenum format, GLenum type) {
		if (!m_RendererID) {
			AXIOM_ERROR("UploadData called on uninitialized texture!");
			return;
		}

		// Prüfe, ob bindless
		if (m_IsBindless) {
			// immutable -> nur SubImage in vorhandener Textur möglich
			if (width > m_Width || height > m_Height) {
				AXIOM_ERROR("UploadData: bindless texture cannot be resized! width={}, height={}", width, height);
				return;
			}

			glTextureSubImage2D(m_RendererID, 0, 0, 0, width, height, format, type, data);
		}
		else if (m_IsImmutable) {
			// normale immutable Textur
			if (width > m_Width || height > m_Height) {
				AXIOM_ERROR("UploadData: immutable texture cannot be resized! width={}, height={}", width, height);
				return;
			}
			glTextureSubImage2D(m_RendererID, 0, 0, 0, width, height, format, type, data);
		}
		else {
			// mutable Textur, kann komplett neu gesetzt werden
			glBindTexture(GL_TEXTURE_2D, m_RendererID);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, format, type, data);
		}

		m_Width = width;
		m_Height = height;

		if (m_Spec.GenerateMipmaps)
			glGenerateTextureMipmap(m_RendererID);
	}

	// ===================== Spec Handling =====================

	void Texture::SetSpec(const TextureSpec& spec) {
		m_Spec = spec;

		ApplyParameters();

		if (m_Spec.GenerateMipmaps)
			glGenerateTextureMipmap(m_RendererID);

		MakeBindless();
	}

	void Texture::SetWrap(TextureWrap s, TextureWrap t) {
		m_Spec.WrapS = s;
		m_Spec.WrapT = t;

		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, ToGLWrap(s));
		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, ToGLWrap(t));
	}

	void Texture::SetFilter(TextureFilter min, TextureFilter mag) {
		m_Spec.MinFilter = min;
		m_Spec.MagFilter = mag;

		glTextureParameteri(
			m_RendererID,
			GL_TEXTURE_MIN_FILTER,
			ToGLFilter(min, m_Spec.GenerateMipmaps)
		);

		glTextureParameteri(
			m_RendererID,
			GL_TEXTURE_MAG_FILTER,
			ToGLFilter(mag)
		);
	}

	void Texture::SetMipmaps(bool enabled) {
		m_Spec.GenerateMipmaps = enabled;

		if (enabled)
			glGenerateTextureMipmap(m_RendererID);

		SetFilter(m_Spec.MinFilter, m_Spec.MagFilter);
	}

	void Texture::ApplyParameters() {
		SetWrap(m_Spec.WrapS, m_Spec.WrapT);
		SetFilter(m_Spec.MinFilter, m_Spec.MagFilter);
	}

	// ===================== Binding =====================

	void Texture::Bind(uint32_t slot) const {
		glBindTextureUnit(slot, m_RendererID);
	}

	void Texture::BindToShader(GLint location, uint32_t slot) const {
		if (!this) {
			AXIOM_ERROR("BindToShader called on nullptr!");
			return;
		}

		if (m_IsBindless) {
			glUniformHandleui64ARB(location, m_Handle);
		}
		else {
			glBindTextureUnit(slot, m_RendererID);
			glUniform1i(location, slot);
		}
	}

	// ===================== Bindless =====================

	void Texture::MakeBindless() {
		CheckBindlessSupport();

		if (!s_BindlessSupported) {
			m_IsImmutable = false;
			return;
		}

		if (m_IsBindless)
			return;

		m_Handle = glGetTextureHandleARB(m_RendererID);
		glMakeTextureHandleResidentARB(m_Handle);
		m_IsBindless = true;
		m_IsImmutable = true;
	}

	void Texture::CheckBindlessSupport() {
		static bool checked = false;
		if (checked) return;
		checked = true;

		s_BindlessSupported = GLAD_GL_ARB_bindless_texture;

		if (s_BindlessSupported)
			AXIOM_INFO("Bindless supported");
		else
			AXIOM_WARN("Bindless NOT supported");
	}

}