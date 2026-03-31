#pragma once
#include <cstdint>
#include <string>

namespace axiom {

    enum class TextureType {
        Texture2D,
        TextureCube,
        Texture3D
    };

    enum class TextureInternalFormat {
        RGBA8,
        RGB8,
        R8
    };

    enum class TextureFormat {
        RGBA,
        RGB,
        RED
    };

    enum class TextureDataType {
        UnsignedByte
    };

    enum class TextureFilter {
        Nearest,
        Linear
    };

    enum class TextureWrap {
        Repeat,
        ClampToEdge
    };

    struct TextureSpecification {
        uint32_t Width = 1;
        uint32_t Height = 1;

        TextureInternalFormat InternalFormat = TextureInternalFormat::RGBA8;
        TextureFormat Format = TextureFormat::RGBA;
        TextureDataType Type = TextureDataType::UnsignedByte;

        TextureFilter MinFilter = TextureFilter::Linear;
        TextureFilter MagFilter = TextureFilter::Linear;

        TextureWrap WrapS = TextureWrap::Repeat;
        TextureWrap WrapT = TextureWrap::Repeat;
        TextureWrap WrapR = TextureWrap::Repeat;

        uint32_t Levels = 1;
    };

    class Texture {
    public:
        Texture(TextureType type, const TextureSpecification& spec);
        ~Texture();

        // Allocation & Data
        void allocate();
        void setData(const void* data);

        // Runtime spec changes
        void setSpecification(const TextureSpecification& spec);
        void setWidth(uint32_t width);
        void setHeight(uint32_t height);
        void setInternalFormat(TextureInternalFormat f);
        void setFormat(TextureFormat f);
        void setDataType(TextureDataType t);
        void setMinFilter(TextureFilter f);
        void setMagFilter(TextureFilter f);
        void setWrapS(TextureWrap w);
        void setWrapT(TextureWrap w);
        void setWrapR(TextureWrap w);

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
