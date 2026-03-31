#include "axiom/renderer/Texture.h"
#include "axiom/core/Logger.h"

#include <stb_image.h>
#include <glad/glad.h>

namespace axiom {

    static GLenum toGL(TextureType t) {
        switch (t) {
        case TextureType::Texture2D: return GL_TEXTURE_2D;
        case TextureType::TextureCube: return GL_TEXTURE_CUBE_MAP;
        case TextureType::Texture3D: return GL_TEXTURE_3D;
        }
        return GL_TEXTURE_2D;
    }

    static GLenum toGL(TextureInternalFormat f) {
        switch (f) {
        case TextureInternalFormat::RGBA8: return GL_RGBA8;
        case TextureInternalFormat::RGB8: return GL_RGB8;
        case TextureInternalFormat::R8: return GL_R8;
        }
        return GL_RGBA8;
    }

    static GLenum toGL(TextureFormat f) {
        switch (f) {
        case TextureFormat::RGBA: return GL_RGBA;
        case TextureFormat::RGB: return GL_RGB;
        case TextureFormat::RED: return GL_RED;
        }
        return GL_RGBA;
    }

    static GLenum toGL(TextureDataType t) {
        switch (t) {
        case TextureDataType::UnsignedByte: return GL_UNSIGNED_BYTE;
        }
        return GL_UNSIGNED_BYTE;
    }

    static GLenum toGL(TextureFilter f) {
        switch (f) {
        case TextureFilter::Nearest: return GL_NEAREST;
        case TextureFilter::Linear: return GL_LINEAR;
        }
        return GL_LINEAR;
    }

    static GLenum toGL(TextureWrap w) {
        switch (w) {
        case TextureWrap::Repeat: return GL_REPEAT;
        case TextureWrap::ClampToEdge: return GL_CLAMP_TO_EDGE;
        }
        return GL_REPEAT;
    }

    // ---------------- Constructor ----------------
    Texture::Texture(TextureType type, const TextureSpecification& spec)
        : m_Type(type), m_Spec(spec) {
        glCreateTextures(toGL(type), 1, &m_RendererID);
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
                toGL(m_Spec.InternalFormat),
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
        glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, toGL(m_Spec.MinFilter));
        glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, toGL(m_Spec.MagFilter));

        glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, toGL(m_Spec.WrapS));
        glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, toGL(m_Spec.WrapT));
        glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_R, toGL(m_Spec.WrapR));
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
                toGL(m_Spec.Format),
                toGL(m_Spec.Type),
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

    // ---------------- Runtime spec changes ----------------
    void Texture::setSpecification(const TextureSpecification& spec) {
        m_Spec = spec;
        if (m_RendererID) {
            allocate();
        }
    }

    void Texture::setWidth(uint32_t width) {
        m_Spec.Width = width;
        if (m_RendererID) allocate();
    }

    void Texture::setHeight(uint32_t height) {
        m_Spec.Height = height;
        if (m_RendererID) allocate();
    }

    void Texture::setInternalFormat(TextureInternalFormat f) {
        m_Spec.InternalFormat = f;
        if (m_RendererID) allocate();
    }

    void Texture::setFormat(TextureFormat f) {
        m_Spec.Format = f;
    }

    void Texture::setDataType(TextureDataType t) {
        m_Spec.Type = t;
    }

    void Texture::setMinFilter(TextureFilter f) {
        m_Spec.MinFilter = f;
        if (m_RendererID) glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, toGL(f));
    }

    void Texture::setMagFilter(TextureFilter f) {
        m_Spec.MagFilter = f;
        if (m_RendererID) glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, toGL(f));
    }

    void Texture::setWrapS(TextureWrap w) {
        m_Spec.WrapS = w;
        if (m_RendererID) glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, toGL(w));
    }

    void Texture::setWrapT(TextureWrap w) {
        m_Spec.WrapT = w;
        if (m_RendererID) glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, toGL(w));
    }

    void Texture::setWrapR(TextureWrap w) {
        m_Spec.WrapR = w;
        if (m_RendererID) glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_R, toGL(w));
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
            m_Spec.InternalFormat = TextureInternalFormat::RGBA8;
            m_Spec.Format = TextureFormat::RGBA;
            m_Spec.Type = TextureDataType::UnsignedByte;
        }
        else if (channels == 3) {
            m_Spec.InternalFormat = TextureInternalFormat::RGB8;
            m_Spec.Format = TextureFormat::RGB;
            m_Spec.Type = TextureDataType::UnsignedByte;
        }
        else if (channels == 1) {
            m_Spec.InternalFormat = TextureInternalFormat::R8;
            m_Spec.Format = TextureFormat::RED;
            m_Spec.Type = TextureDataType::UnsignedByte;
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
