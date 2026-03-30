#include "axiom/renderer/Texture.h"
#include "axiom/core/Logger.h"

#include <stb_image.h>

namespace axiom {

    // ---------------- Constructor ----------------
    Texture::Texture(TextureType type, const TextureSpecification& spec)
        : m_Type(type), m_Spec(spec) {
        glCreateTextures((GLenum)m_Type, 1, &m_RendererID);
    }

    // ---------------- Destructor ----------------
    Texture::~Texture() {
        if (m_RendererID)
            glDeleteTextures(1, &m_RendererID);
    }

    // ---------------- Allocate ----------------
    void Texture::allocate() {
        switch (m_Type) {
        case TextureType::Texture2D:
        {
            glTextureStorage2D(
                m_RendererID,
                m_Spec.Levels,
                m_Spec.InternalFormat,
                m_Spec.Width,
                m_Spec.Height
            );
            break;
        }

        // später erweiterbar:
        case TextureType::Texture3D:
        case TextureType::TextureCube:
        default:
        {
            AXIOM_ASSERT(false, "Texture type not implemented yet");
        }
        }

        // Sampler Settings
        glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, m_Spec.MinFilter);
        glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, m_Spec.MagFilter);

        glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, m_Spec.WrapS);
        glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, m_Spec.WrapT);
        glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_R, m_Spec.WrapR);
    }

    // ---------------- Set Data ----------------
    void Texture::setData(const void* data) {
        switch (m_Type) {
        case TextureType::Texture2D:
        {
            glTextureSubImage2D(
                m_RendererID,
                0,
                0, 0,
                m_Spec.Width,
                m_Spec.Height,
                m_Spec.Format,
                m_Spec.Type,
                data
            );
            break;
        }

        default:
        {
            AXIOM_ASSERT(false, "setData not implemented for this texture type");
        }
        }
    }

    // ---------------- Load from File ----------------
    void Texture::loadFromFile(const std::string& path, bool flip) {
        stbi_set_flip_vertically_on_load(flip);

        int width, height, channels;
        unsigned char* data = stbi_load(path.c_str(), &width, &height, &channels, 0);

        AXIOM_ASSERT(data, "Failed to load texture: {}", path);

        m_Spec.Width = width;
        m_Spec.Height = height;
        m_Spec.Levels = 1;

        if (channels == 4) {
            m_Spec.InternalFormat = GL_RGBA8;
            m_Spec.Format = GL_RGBA;
            m_Spec.Type = GL_UNSIGNED_BYTE;
        }
        else if (channels == 3) {
            m_Spec.InternalFormat = GL_RGB8;
            m_Spec.Format = GL_RGB;
            m_Spec.Type = GL_UNSIGNED_BYTE;
        }
        else if (channels == 1) {
            m_Spec.InternalFormat = GL_R8;
            m_Spec.Format = GL_RED;
            m_Spec.Type = GL_UNSIGNED_BYTE;
        }
        else {
            AXIOM_ASSERT(false, "Unsupported texture format");
        }

        allocate();
        setData(data);

        glGenerateTextureMipmap(m_RendererID);

        stbi_image_free(data);

        AXIOM_INFO("Loaded texture: {} ({}x{}, {} channels)", path, width, height, channels);
    }

    // ---------------- Bind ----------------
    void Texture::bind(uint32_t slot) const {
        glBindTextureUnit(slot, m_RendererID);
    }

}