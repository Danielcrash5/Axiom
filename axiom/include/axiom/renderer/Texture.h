#pragma once
#include "TextureSpec.h"
#include <string>
#include <glad/glad.h>

namespace axiom {

    class Texture {
    public:
        Texture(const std::string& path, const TextureSpec& spec = TextureSpec());
        Texture(uint32_t width, uint32_t height, const TextureSpec& spec = TextureSpec());
        ~Texture();

        // ===== Binding =====
        void Bind(uint32_t slot = 0) const;
        void BindToShader(GLint location, uint32_t slot = 0) const;

        // ===== Spec komplett =====
        void SetSpec(const TextureSpec& spec);
        const TextureSpec& GetSpec() const {
            return m_Spec;
        }

        // ===== Einzelne Specs =====
        void SetWrap(TextureWrap s, TextureWrap t);
        void SetFilter(TextureFilter min, TextureFilter mag);
        void SetMipmaps(bool enabled);

        // ===== Bindless =====
        bool IsBindless() const {
            return m_IsBindless;
        }
        GLuint64 GetHandle() const {
            return m_Handle;
        }

        uint32_t GetRendererID() {
            return m_RendererID;
        }

        void SetRendererID(uint32_t ID) {
            m_RendererID = ID;
        }

        void UploadData(void* data, uint32_t width, uint32_t height, GLenum format = GL_RGBA, GLenum type = GL_UNSIGNED_BYTE);

    private:
        void LoadFromFile(const std::string& path);
        void CreateEmpty();

        void ApplyParameters();
        void MakeBindless();

        static void CheckBindlessSupport();

    private:
        uint32_t m_RendererID = 0;
        uint32_t m_Width = 0, m_Height = 0;
        bool m_IsImmutable;

        TextureSpec m_Spec;

        GLuint64 m_Handle = 0;
        bool m_IsBindless = false;

        static bool s_BindlessSupported;
    };

}