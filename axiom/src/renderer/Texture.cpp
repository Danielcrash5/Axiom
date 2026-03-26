#include "axiom/renderer/Texture.h"
#include "axiom/core/Logger.h"

namespace axiom {
    Texture::Texture(int w, int h, GLenum internalFormat)
        : width(w), height(h) {
        glCreateTextures(GL_TEXTURE_2D, 1, &textureID);
        glTextureStorage2D(textureID, 1, internalFormat, width, height);
        SetWrap(GL_REPEAT, GL_REPEAT);
        SetFilter(GL_LINEAR, GL_LINEAR);
        // noch keine Bindless Textures da ich zu Faul bin den richtigen Shader zu schreiben MakeBindless();
    }

    Texture::Texture(const std::string& filePath, bool flipVertically) {
        stbi_set_flip_vertically_on_load(flipVertically);
        int channels;
        unsigned char* data = stbi_load(filePath.c_str(), &width, &height, &channels, 4);

        AXIOM_ASSERT(data, "Failed to load texture {}", filePath);

        glCreateTextures(GL_TEXTURE_2D, 1, &textureID);
        glTextureStorage2D(textureID, 1, GL_RGBA8, width, height);
        glTextureSubImage2D(textureID, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);

        SetWrap(GL_REPEAT, GL_REPEAT);
        SetFilter(GL_LINEAR, GL_LINEAR);

        stbi_image_free(data);
        // noch keine Bindless Textures da ich zu Faul bin den richtigen Shader zu schreiben MakeBindless();
    }

    Texture::~Texture() {
        if (isBindless)
            glMakeTextureHandleNonResidentARB(bindlessHandle);
        if (textureID)
            glDeleteTextures(1, &textureID);
    }

    // ===================== Klassisches Binden =====================
    void Texture::Bind(GLuint unit) const {
        glBindTextureUnit(unit, textureID);
    }

    // ===================== Bindless Textures =====================
    void Texture::MakeBindless() {
        CheckBindlessSupport();

        if (!s_BindlessSupported) {
            AXIOM_ERROR("[Texture] Cannot make bindless: not supported on this GPU.");
            return;
        }

        if (isBindless) return;

        bindlessHandle = glGetTextureHandleARB(textureID);
        glMakeTextureHandleResidentARB(bindlessHandle);
        isBindless = true;
    }

    // ===================== Shader-Kompatible Bindung =====================
    void Texture::BindToShader(GLint location, GLuint textureSlot) const {
        if (isBindless) {
            glUniformHandleui64ARB(location, bindlessHandle);
        } else {
            glActiveTexture(GL_TEXTURE0 + textureSlot);
            glBindTextureUnit(textureSlot, textureID);
            glUniform1i(location, textureSlot);
        }
    }

    // ===================== Parameter =====================
    void Texture::SetWrap(GLenum s, GLenum t) {
        glTextureParameteri(textureID, GL_TEXTURE_WRAP_S, s);
        glTextureParameteri(textureID, GL_TEXTURE_WRAP_T, t);
    }

    void Texture::SetFilter(GLenum minFilter, GLenum magFilter) {
        glTextureParameteri(textureID, GL_TEXTURE_MIN_FILTER, minFilter);
        glTextureParameteri(textureID, GL_TEXTURE_MAG_FILTER, magFilter);
    }

    bool Texture::s_BindlessSupported = false;

    void Texture::CheckBindlessSupport() {
        static bool checked = false;
        if (checked) return;
        checked = true;

        // Prüfen, ob Extension vorhanden
        if (GLAD_GL_ARB_bindless_texture) {
            s_BindlessSupported = true;
            AXIOM_INFO("[Texture] Bindless Textures supported.");
        } else {
            s_BindlessSupported = false;
            AXIOM_INFO("[Texture] Bindless Textures NOT supported.");
        }
    }
}