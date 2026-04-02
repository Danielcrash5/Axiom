#pragma once
#include "axiom/renderer/Shader.h"
#include "axiom/renderer/ShaderSource.h"

class OpenGLShader : public Shader {
public:
    OpenGLShader(const std::string& path);
    ~OpenGLShader();

    void Bind() const override;
    void Unbind() const override;

    void SetFloat(const std::string& name, float value) override;

private:
    uint32_t m_ID;

    void Compile(const ShaderSource& sources);
};