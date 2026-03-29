#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include <glm/glm.hpp>

enum class ShaderStage {
    Vertex,
    Fragment,
    Geometry,
    TessControl,
    TessEval,
    Compute
};

struct ShaderStageSource {
    ShaderStage stage;
    std::string path;
};

class Shader {
public:
    Shader();   // leer, noch kein Shader-Programm
    ~Shader();

    // Compile Shader mit Stages
    void compile(const std::vector<ShaderStageSource>& stages);

    void bind() const;
    void unbind() const;

    // Uniforms
    void setInt(const std::string& name, int value);
    void setFloat(const std::string& name, float value);
    void setVec2(const std::string& name, const glm::vec2& value);
    void setVec3(const std::string& name, const glm::vec3& value);
    void setVec4(const std::string& name, const glm::vec4& value);
    void setMat4(const std::string& name, const glm::mat4& value);

    // Feature Flags
    void setDefine(const std::string& define, const std::string& value = "1");

private:
    unsigned int m_RendererID = 0;
    std::unordered_map<std::string, int> m_UniformLocationCache;
    std::unordered_map<std::string, std::string> m_Defines;

    std::string parseShaderFile(const std::string& path);
    std::string preprocessShader(const std::string& source, const std::string& parentDir);
    unsigned int compileShader(unsigned int type, const std::string& source);
    int getUniformLocation(const std::string& name);
    void introspectUniforms();

    unsigned int shaderStageToGL(ShaderStage stage);
};