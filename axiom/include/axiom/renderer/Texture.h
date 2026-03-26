#pragma once
#include <glad/glad.h>
#include <string>
#include <memory>
#include <stb_image.h>

namespace axiom {
    class Texture {
    public:
        // ===================== Konstruktoren =====================
        Texture(int width, int height, GLenum internalFormat = GL_RGBA8);
        Texture(const std::string& filePath, bool flipVertically = true);
        ~Texture();

        // ===================== Klassisches Binden =====================
        void Bind(GLuint unit = 0) const;

        // ===================== Bindless Textures =====================
        void MakeBindless();
        bool IsBindless() const { return isBindless; }
        GLuint64 GetBindlessHandle() const { return bindlessHandle; }

        // ===================== Shader-Kompatible Bindung =====================
        void BindToShader(GLint location, GLuint textureSlot = 0) const;

        // ===================== Parameter =====================
        void SetWrap(GLenum s, GLenum t);
        void SetFilter(GLenum minFilter, GLenum magFilter);

        // ===================== Getter =====================
        int GetWidth() const { return width; }
        int GetHeight() const { return height; }
        GLuint GetID() const { return textureID; }

    private:
        GLuint textureID = 0;
        GLuint64 bindlessHandle = 0;
        bool isBindless = false;
        int width = 0;
        int height = 0;

        static bool s_BindlessSupported; // Ein Flag für die ganze Klasse
        static void CheckBindlessSupport();

        void UploadTexture(unsigned char* data, int w, int h, GLenum format);
    };
}
