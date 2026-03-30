#pragma once
#include <cstdint>
#include <string>
#include <glad/glad.h>

namespace axiom {

    enum class TextureType {
        Texture2D = GL_TEXTURE_2D,
        TextureCube = GL_TEXTURE_CUBE_MAP,
        Texture3D = GL_TEXTURE_3D
    };

    struct TextureSpecification {
        uint32_t Width = 1;
        uint32_t Height = 1;

        uint32_t InternalFormat = GL_RGBA8;
        uint32_t Format = GL_RGBA;
        uint32_t Type = GL_UNSIGNED_BYTE;

        uint32_t MinFilter = GL_LINEAR;
        uint32_t MagFilter = GL_LINEAR;

        uint32_t WrapS = GL_REPEAT;
        uint32_t WrapT = GL_REPEAT;
        uint32_t WrapR = GL_REPEAT;

        uint32_t Levels = 1;
    };

    class Texture {
    public:
        Texture(TextureType type, const TextureSpecification& spec);
        ~Texture();

        // Allocation & Data
        void allocate();
        void setData(const void* data);

        // File loading
        void loadFromFile(const std::string& path, bool flip = true);

        // Bind
        void bind(uint32_t slot = 0) const;

        // Getters
        uint32_t getRendererID() const {
            return m_RendererID;
        }
        const TextureSpecification& getSpecification() const {
            return m_Spec;
        }

    private:
        uint32_t m_RendererID = 0;
        TextureType m_Type;
        TextureSpecification m_Spec;
    };

}