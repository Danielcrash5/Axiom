#include "axiom/renderer/Shader.h"
#include <glad/glad.h>
#include <fstream>
#include <sstream>
#include "axiom/core/Logger.h"
#include <filesystem>

Shader::Shader() = default;

Shader::~Shader() {
    if (m_RendererID != 0)
        glDeleteProgram(m_RendererID);
}

// ---------------- Compile Shader ----------------
void Shader::compile(const std::vector<ShaderStageSource>& stages) {
    if (m_RendererID != 0)
        glDeleteProgram(m_RendererID); // alten Shader löschen

    m_RendererID = glCreateProgram();

    std::vector<unsigned int> compiledShaders;

    for (auto& stage : stages) {
        std::filesystem::path path(stage.path);
        std::string src = preprocessShader(parseShaderFile(stage.path), path.parent_path().string());
        unsigned int shader = compileShader(shaderStageToGL(stage.stage), src);
        glAttachShader(m_RendererID, shader);
        compiledShaders.push_back(shader);
    }

    glLinkProgram(m_RendererID);

    int success;
    glGetProgramiv(m_RendererID, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[1024];
        glGetProgramInfoLog(m_RendererID, 1024, nullptr, infoLog);
        AXIOM_ERROR("Shader Program Linking Failed:\n{}", infoLog);
    }

    for (auto shader : compiledShaders)
        glDeleteShader(shader);

    introspectUniforms();
}

// ---------------- Bind / Unbind ----------------
void Shader::bind() const {
    glUseProgram(m_RendererID);
}
void Shader::unbind() const {
    glUseProgram(0);
}

// ---------------- Defines ----------------
void Shader::setDefine(const std::string& define, const std::string& value) {
    m_Defines[define] = value;
}

// ---------------- Preprocess Shader ----------------
std::string Shader::preprocessShader(const std::string& source, const std::string& parentDir) {
    std::stringstream ss;

    // füge alle definierten Flags ein
    for (auto& [key, val] : m_Defines)
        ss << "#define " << key << " " << val << "\n";

    std::istringstream stream(source);
    std::string line;

    while (std::getline(stream, line)) {
        if (line.find("#include") != std::string::npos) {
            size_t firstQuote = line.find('"');
            size_t lastQuote = line.find_last_of('"');
            AXIOM_ASSERT(firstQuote != std::string::npos && lastQuote != std::string::npos,
                         "Malformed #include directive");

            std::string includeFile = line.substr(firstQuote + 1, lastQuote - firstQuote - 1);
            std::string includePath = parentDir + "/" + includeFile;

            std::string includedSource = preprocessShader(parseShaderFile(includePath), parentDir);
            ss << includedSource << "\n";
        }
        else {
            ss << line << "\n";
        }
    }

    return ss.str();
}

// ---------------- Parse Shader File ----------------
std::string Shader::parseShaderFile(const std::string& path) {
    std::ifstream stream(path);
    AXIOM_ASSERT(stream.is_open(), "Failed to open shader file: {}", path);

    std::stringstream ss;
    ss << stream.rdbuf();
    return ss.str();
}

// ---------------- Compile single shader ----------------
unsigned int Shader::compileShader(unsigned int type, const std::string& source) {
    unsigned int id = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(id, 1, &src, nullptr);
    glCompileShader(id);

    int success;
    glGetShaderiv(id, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[1024];
        glGetShaderInfoLog(id, 1024, nullptr, infoLog);
        AXIOM_ERROR("Shader Compilation Failed ({}):\n{}", type, infoLog);
    }

    return id;
}

// ---------------- Uniform Introspection ----------------
void Shader::introspectUniforms() {
    m_UniformLocationCache.clear();

    int uniformCount;
    glGetProgramiv(m_RendererID, GL_ACTIVE_UNIFORMS, &uniformCount);

    char nameBuffer[256];
    for (int i = 0; i < uniformCount; i++) {
        GLenum type;
        int size;
        glGetActiveUniform(m_RendererID, i, 256, nullptr, &size, &type, nameBuffer);
        std::string name(nameBuffer);

        m_UniformLocationCache[name] = glGetUniformLocation(m_RendererID, name.c_str());
        AXIOM_DEBUG("Shader Uniform Found: {} (size: {}, type: {})", name, size, type);
    }
}

// ---------------- Get Uniform Location ----------------
int Shader::getUniformLocation(const std::string& name) {
    if (m_UniformLocationCache.find(name) != m_UniformLocationCache.end())
        return m_UniformLocationCache[name];

    AXIOM_WARN("Uniform '{}' does not exist in shader", name);
    return -1;
}

// ---------------- Uniform Set Functions ----------------
void Shader::setInt(const std::string& name, int value) {
    glUniform1i(getUniformLocation(name), value);
}
void Shader::setFloat(const std::string& name, float value) {
    glUniform1f(getUniformLocation(name), value);
}
void Shader::setVec2(const std::string& name, const glm::vec2& value) {
    glUniform2fv(getUniformLocation(name), 1, &value[0]);
}
void Shader::setVec3(const std::string& name, const glm::vec3& value) {
    glUniform3fv(getUniformLocation(name), 1, &value[0]);
}
void Shader::setVec4(const std::string& name, const glm::vec4& value) {
    glUniform4fv(getUniformLocation(name), 1, &value[0]);
}
void Shader::setMat4(const std::string& name, const glm::mat4& value) {
    glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE, &value[0][0]);
}

// ---------------- ShaderStage to GL ----------------
unsigned int Shader::shaderStageToGL(ShaderStage stage) {
    switch (stage) {
    case ShaderStage::Vertex: return GL_VERTEX_SHADER;
    case ShaderStage::Fragment: return GL_FRAGMENT_SHADER;
    case ShaderStage::Geometry: return GL_GEOMETRY_SHADER;
    case ShaderStage::TessControl: return GL_TESS_CONTROL_SHADER;
    case ShaderStage::TessEval: return GL_TESS_EVALUATION_SHADER;
    case ShaderStage::Compute: return GL_COMPUTE_SHADER;
    }
    AXIOM_ERROR("Unknown ShaderStage enum");
    return 0;
}