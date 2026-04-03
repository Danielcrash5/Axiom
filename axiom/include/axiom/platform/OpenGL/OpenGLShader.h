#pragma once
#include "axiom/renderer/Shader.h"
#include <glad/glad.h>
#include <unordered_map>

class OpenGLShader : public Shader {
public:
    OpenGLShader(
        const std::string& path,
        const std::unordered_set<std::string>& defines
    );

    ~OpenGLShader();

    void Bind() const override;
    void Unbind() const override;

    void SetFloat(const std::string& name, float value) override;

private:
    uint32_t m_RendererID;

    void Compile(const std::unordered_map<GLenum, std::string>& shaders);
    void Reflect();

    std::unordered_map<GLenum, std::string> Parse(const std::string& source);
};