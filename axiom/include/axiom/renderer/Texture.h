#pragma once
#include "TextureSpec.h"
#include <string>
#include <glad/glad.h>

namespace axiom {

    class Texture {
    public:
        Texture(const std::string& path, const TextureSpec& spec = {});
        Texture(uint32_t width, uint32_t height, const TextureSpec& spec = {});
        ~Texture();

        // ===== Upload =====
        void UploadData(const void* data, uint32_t width, uint32_t height);

        // ===== Binding =====
        void Bind(uint32_t slot = 0) const;
        void BindToShader(GLint location, uint32_t slot = 0) const;

        // ===== Info =====
        uint32_t GetRendererID() const {
            return m_RendererID;
        }
        bool IsBindless() const {
            return m_IsBindless;
        }

    private:
        void Create(uint32_t width, uint32_t height, GLenum internalFormat);
        void LoadFromFile(const std::string& path);
        void ApplyParameters();

        void MakeBindless();
        static void CheckBindlessSupport();

    private:
        uint32_t m_RendererID = 0;
        uint32_t m_Width = 0;
        uint32_t m_Height = 0;

        GLenum m_InternalFormat = GL_RGBA8;
        GLenum m_DataFormat = GL_RGBA;

        TextureSpec m_Spec;

        // Bindless
        bool m_IsBindless = false;
        GLuint64 m_Handle = 0;

        static bool s_BindlessSupported;
    };

}