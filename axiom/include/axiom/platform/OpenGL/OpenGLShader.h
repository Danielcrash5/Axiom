#pragma once
#include "axiom/renderer/Shader.h"
#include "axiom/core/Logger.h"
#include <glad/glad.h>
#include <unordered_map>
#include <stdexcept>

struct UniformInfo {
    GLint Location;
    GLenum Type;
    GLint Size;
    bool IsTexture; // true wenn Sampler
};

class OpenGLShader : public Shader {
public:
    OpenGLShader(
        const std::string& path,
        const std::unordered_set<std::string>& defines
    );

    ~OpenGLShader();

    void Bind() const override;
    void Unbind() const override;

    void SetUniform1i(const std::string& name, int value) override {
        glUniform1i(GetUniformLocation(name), value);
    }

    void SetUniform1f(const std::string& name, float value) override {
        glUniform1f(GetUniformLocation(name), value);
    }

    void SetUniform2f(const std::string& name, const glm::vec2& value) override {
        glUniform2f(GetUniformLocation(name), value.x, value.y);
    }

    void SetUniform3f(const std::string& name, const glm::vec3& value) override {
        glUniform3f(GetUniformLocation(name), value.x, value.y, value.z);
    }

    void SetUniform4f(const std::string& name, const glm::vec4& value) override {
        glUniform4f(GetUniformLocation(name), value.x, value.y, value.z, value.w);
    }

    void SetUniformMat3(const std::string& name, const glm::mat3& value) override {
        glUniformMatrix3fv(GetUniformLocation(name), 1, GL_FALSE, &value[0][0]);
    }

    void SetUniformMat4(const std::string& name, const glm::mat4& value) override {
        glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, &value[0][0]);
    }

    void SetTexture(const std::string& name, const std::shared_ptr<axiom::Texture2D>& texture, uint32_t slot) override {
        auto it = m_UniformLocationCache.find(name);
        if (it == m_UniformLocationCache.end()) {
            AXIOM_ERROR("[Shader] Uniform not found: {}", name);
            return;
        }

        GLint location = it->second.Location;
        GLenum type = it->second.Type;

        if (!texture)
            return;

        if (texture->SupportsBindless()) {
            // Bindless Textures
            GLuint64 handle = texture->GetBindlessHandle();
            glUniformHandleui64ARB(location, handle);
        }
        else {
            // Standard Texture Binding
            glActiveTexture(GL_TEXTURE0 + slot);
            texture->Bind(slot);

            // Sampler uniform muss den Slot setzen
            glUniform1i(location, slot);
        }
    }
private:
    uint32_t m_RendererID;

    mutable std::unordered_map<std::string, UniformInfo> m_UniformLocationCache;

    int GetUniformLocation(const std::string& name) const {
        if (m_UniformLocationCache.find(name) != m_UniformLocationCache.end())
            return m_UniformLocationCache[name].Location;
        else
            throw std::runtime_error("Unknown Uniform");
    }

    void Compile(const std::unordered_map<GLenum, std::string>& shaders);
    void Reflect();

    std::unordered_map<GLenum, std::string> Parse(const std::string& source);
};