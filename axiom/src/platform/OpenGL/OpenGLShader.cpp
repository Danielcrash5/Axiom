#include "axiom/platform/OpenGL/OpenGLShader.h"
#include "axiom/renderer/SlangCompiler.h"
#include <glad/glad.h>
#include <iostream>

static GLenum ToGL(ShaderStage stage) {
    switch (stage) {
    case ShaderStage::Vertex: return GL_VERTEX_SHADER;
    case ShaderStage::Fragment: return GL_FRAGMENT_SHADER;
    case ShaderStage::Geometry: return GL_GEOMETRY_SHADER;
    case ShaderStage::Compute: return GL_COMPUTE_SHADER;
    case ShaderStage::TessControl: return GL_TESS_CONTROL_SHADER;
    case ShaderStage::TessEvaluation: return GL_TESS_EVALUATION_SHADER;
    }
    return 0;
}

OpenGLShader::OpenGLShader(const std::string& path) {
    auto sources = CompileSlang(path);
    Compile(sources);
}

OpenGLShader::~OpenGLShader() {
    glDeleteProgram(m_ID);
}

void OpenGLShader::Compile(const ShaderSource& sources) {
    GLuint program = glCreateProgram();
    std::vector<GLuint> shaders;

    for (auto& [stage, src] : sources.Sources) {
        GLuint shader = glCreateShader(ToGL(stage));
        const char* code = src.c_str();

        glShaderSource(shader, 1, &code, nullptr);
        glCompileShader(shader);

        int success;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

        if (!success) {
            char log[1024];
            glGetShaderInfoLog(shader, 1024, nullptr, log);
            std::cout << log << std::endl;
        }

        glAttachShader(program, shader);
        shaders.push_back(shader);
    }

    glLinkProgram(program);

    for (auto s : shaders)
        glDeleteShader(s);

    m_ID = program;
}

void OpenGLShader::Bind() const {
    glUseProgram(m_ID);
}

void OpenGLShader::Unbind() const {
    glUseProgram(0);
}

void OpenGLShader::SetFloat(const std::string& name, float value) {
    GLint loc = glGetUniformLocation(m_ID, name.c_str());
    glUniform1f(loc, value);
}