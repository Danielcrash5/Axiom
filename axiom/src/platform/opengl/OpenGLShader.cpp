#include "axiom/platform/opengl/OpenGLShader.h"
#include "axiom/core/Logger.h"

#include <glad/glad.h>

#include <fstream>
#include <sstream>
#include <iostream>

namespace axiom {

    // ===================== CONSTRUCTOR =====================

    OpenGLShader::OpenGLShader(const ShaderDesc& desc) {
        m_ProgramID = glCreateProgram();
        std::vector<uint32_t> shaders;

        auto attach = [&](const std::string& path, GLenum type)
            {
                if (path.empty())
                    return;

                std::string source = ReadFileWithIncludes(path);

                uint32_t shader = CompileStage(type, source);

                glAttachShader(m_ProgramID, shader);
                shaders.push_back(shader);
            };

        attach(desc.Vertex, GL_VERTEX_SHADER);
        attach(desc.Fragment, GL_FRAGMENT_SHADER);
        attach(desc.Geometry, GL_GEOMETRY_SHADER);
        attach(desc.Compute, GL_COMPUTE_SHADER);
        attach(desc.TessControl, GL_TESS_CONTROL_SHADER);
        attach(desc.TessEvaluation, GL_TESS_EVALUATION_SHADER);

        glLinkProgram(m_ProgramID);

        int success;
        glGetProgramiv(m_ProgramID, GL_LINK_STATUS, &success);

        if (!success) {
            char info[1024];
            glGetProgramInfoLog(m_ProgramID, 1024, nullptr, info);
            AXIOM_ERROR("Shader linking failed: {}", info);
        }

        for (auto s : shaders)
            glDeleteShader(s);

        Reflect();
    }

    // ===================== DESTRUCTOR =====================

    OpenGLShader::~OpenGLShader() {
        glDeleteProgram(m_ProgramID);
    }

    // ===================== BIND =====================

    void OpenGLShader::Bind() const {
        glUseProgram(m_ProgramID);
    }

    // ===================== FILE LOADING =====================

    std::string OpenGLShader::ReadFileWithIncludes(const std::string& path) {
        std::ifstream file(path);

        if (!file.is_open()) {
            AXIOM_ERROR("Failed to open shader file: {}", path);
            return "";
        }

        std::stringstream ss;
        ss << file.rdbuf();

        std::string source = ss.str();

        std::unordered_set<std::string> includedFiles;

        size_t lastSlash = path.find_last_of("/\\");
        std::string directory = (lastSlash == std::string::npos) ? "" : path.substr(0, lastSlash + 1);

        return ProcessIncludes(source, directory, includedFiles);
    }

    // ===================== INCLUDE SYSTEM =====================

    std::string OpenGLShader::ProcessIncludes(
        const std::string& source,
        const std::string& directory,
        std::unordered_set<std::string>& includedFiles) {
        std::stringstream output;
        std::istringstream input(source);

        std::string line;

        while (std::getline(input, line)) {
            if (line.find("#include") != std::string::npos) {
                size_t start = line.find("\"");
                size_t end = line.find("\"", start + 1);

                if (start == std::string::npos || end == std::string::npos) {
                    AXIOM_ERROR("Invalid include syntax: {}", line);
                    continue;
                }

                std::string includePath = line.substr(start + 1, end - start - 1);
                std::string fullPath = directory + includePath;

                if (includedFiles.contains(fullPath)) {
                    AXIOM_ERROR("Include loop detected: {}", fullPath);
                    continue;
                }

                includedFiles.insert(fullPath);

                std::ifstream incFile(fullPath);
                if (!incFile.is_open()) {
                    AXIOM_ERROR("Failed to open include file: {}", fullPath);
                    continue;
                }

                std::stringstream incStream;
                incStream << incFile.rdbuf();

                std::string includedSource = ProcessIncludes(
                    incStream.str(),
                    directory,
                    includedFiles
                );

                output << "\n// --- Begin include: " << includePath << " ---\n";
                output << includedSource;
                output << "\n// --- End include: " << includePath << " ---\n";
            } else {
                output << line << "\n";
            }
        }

        return output.str();
    }

    // ===================== COMPILE =====================

    uint32_t OpenGLShader::CompileStage(uint32_t type, const std::string& source) {
        uint32_t shader = glCreateShader(type);

        const char* src = source.c_str();
        glShaderSource(shader, 1, &src, nullptr);
        glCompileShader(shader);

        int success;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

        if (!success) {
            char info[1024];
            glGetShaderInfoLog(shader, 1024, nullptr, info);
            AXIOM_ERROR("Shader compilation failed: {}", info);
        }

        return shader;
    }

    // ===================== REFLECTION =====================

    void OpenGLShader::Reflect() {
        int count;
        glGetProgramiv(m_ProgramID, GL_ACTIVE_UNIFORMS, &count);

        for (int i = 0; i < count; i++) {
            char name[256];
            GLsizei length;
            GLint size;
            GLenum type;

            glGetActiveUniform(m_ProgramID, i, sizeof(name),
                               &length, &size, &type, name);

            int location = glGetUniformLocation(m_ProgramID, name);

            ShaderResource res;
            res.name = name;
            res.location = location;
            res.binding = 0;

            if (type == GL_SAMPLER_2D)
                res.type = ShaderResourceType::Sampler2D;
            else
                res.type = ShaderResourceType::Uniform;

            m_Resources.push_back(res);
        }
    }

}