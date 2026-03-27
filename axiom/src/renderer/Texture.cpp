#include "axiom/renderer/Texture.h"
#include "axiom/core/Logger.h"
#include <stb_image.h>

namespace axiom {

    bool Texture::s_BindlessSupported = false;

    // ===================== Utils =====================

    static GLenum ToGLWrap(TextureWrap wrap) {
        switch (wrap) {
        case TextureWrap::Repeat: return GL_REPEAT;
        case TextureWrap::Clamp:  return GL_CLAMP_TO_EDGE;
        case TextureWrap::Mirror: return GL_MIRRORED_REPEAT;
        }
        return GL_REPEAT;
    }

    static GLenum ToGLFilter(TextureFilter filter, bool mipmaps) {
        if (mipmaps)
            return (filter == TextureFilter::Linear) ? GL_LINEAR_MIPMAP_LINEAR : GL_NEAREST_MIPMAP_NEAREST;
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
        : m_Spec(spec) {
        Create(width, height, GL_RGBA8);
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

    void Texture::Create(uint32_t width, uint32_t height, GLenum internalFormat) {
        m_Width = width;
        m_Height = height;
        m_InternalFormat = internalFormat;

        glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
        glTextureStorage2D(m_RendererID, 1, internalFormat, width, height);

        m_DataFormat = GL_RGBA; // default safe fallback
    }

    void Texture::LoadFromFile(const std::string& path) {
        stbi_set_flip_vertically_on_load(m_Spec.FlipOnLoad);

        int channels;
        unsigned char* data = stbi_load(path.c_str(), (int*)&m_Width, (int*)&m_Height, &channels, 0);

        AXIOM_ASSERT(data, "Failed to load texture {}", path);

        m_DataFormat = ChannelsToFormat(channels);

        m_InternalFormat =
            (m_DataFormat == GL_RGB) ? GL_RGB8 :
            (m_DataFormat == GL_RED) ? GL_R8 : GL_RGBA8;

        Create(m_Width, m_Height, m_InternalFormat);

        glTextureSubImage2D(
            m_RendererID,
            0,
            0, 0,
            m_Width,
            m_Height,
            m_DataFormat,
            GL_UNSIGNED_BYTE,
            data
        );

        if (m_Spec.GenerateMipmaps)
            glGenerateTextureMipmap(m_RendererID);

        stbi_image_free(data);
    }

    // ===================== Upload =====================

    void Texture::UploadData(const void* data, uint32_t width, uint32_t height) {
        AXIOM_ASSERT(data, "UploadData: data is null!");
        AXIOM_ASSERT(m_RendererID != 0, "Texture not initialized!");

        if (m_IsImmutable) {
            AXIOM_ASSERT(width <= m_Width && height <= m_Height,
                         "Cannot resize immutable texture!");
            glTextureSubImage2D(
                m_RendererID,
                0,
                0, 0,
                width,
                height,
                m_DataFormat,
                GL_UNSIGNED_BYTE,
                data
            );
        }
        else {
            // Normale Textur → resize erlaubt
            glBindTexture(GL_TEXTURE_2D, m_RendererID);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0,
                         m_DataFormat, GL_UNSIGNED_BYTE, data);
        }

        if (m_Spec.GenerateMipmaps)
            glGenerateTextureMipmap(m_RendererID);

        m_Width = width;
        m_Height = height;
    }

    // ===================== Parameters =====================

    void Texture::ApplyParameters() {
        glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, ToGLWrap(m_Spec.WrapS));
        glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, ToGLWrap(m_Spec.WrapT));

        glTextureParameteri(
            m_RendererID,
            GL_TEXTURE_MIN_FILTER,
            ToGLFilter(m_Spec.MinFilter, m_Spec.GenerateMipmaps)
        );

        glTextureParameteri(
            m_RendererID,
            GL_TEXTURE_MAG_FILTER,
            ToGLFilter(m_Spec.MagFilter, false)
        );

        // Mipmaps
        if (m_Spec.GenerateMipmaps)
            glGenerateTextureMipmap(m_RendererID);

        // Anisotropes Filtern
        if (m_Spec.AnisotropicFiltering) {
            if (GLAD_GL_EXT_texture_filter_anisotropic) {
                GLfloat maxAniso = 0.0f;
                glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAniso);
                glTextureParameterf(m_RendererID, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAniso);
            }
            else {
                AXIOM_WARN("Anisotropic filtering requested but not supported by GPU");
            }
        }
    }

    // ===================== Binding =====================

    void Texture::Bind(uint32_t slot) const {
        glBindTextureUnit(slot, m_RendererID);
    }

    void Texture::BindToShader(GLint location, uint32_t slot) const {
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

        if (!s_BindlessSupported)
            return;

        if (m_IsBindless)
            return;

        m_Handle = glGetTextureHandleARB(m_RendererID);
        glMakeTextureHandleResidentARB(m_Handle);

        m_IsBindless = true;
    }

    void Texture::CheckBindlessSupport() {
        static bool checked = false;
        if (checked) return;
        checked = true;

        s_BindlessSupported = GLAD_GL_ARB_bindless_texture;

        if (s_BindlessSupported)
            AXIOM_INFO("Bindless textures supported");
        else
            AXIOM_WARN("Bindless textures NOT supported");
    }

}