#include "axiom/platform/OpenGL/OpenGLShader.h"
#include "axiom/renderer/ShaderPreprocessor.h"

#include <fstream>
#include <sstream>
#include <iostream>
namespace axiom {
	static std::string ReadFile(const std::string& path) {
		std::ifstream file(path);
		std::stringstream ss;
		ss << file.rdbuf();
		return ss.str();
	}

	OpenGLShader::OpenGLShader(
		const std::string& path,
		const std::unordered_set<std::string>& defines) {
		std::string source = ReadFile(path);

		std::string dir = path.substr(0, path.find_last_of("/\\"));

		source = ShaderPreprocessor::Process(source, dir, defines);

		auto shaders = Parse(source);

		Compile(shaders);
		Reflect();
	}

	OpenGLShader::~OpenGLShader() {
		glDeleteProgram(m_RendererID);
	}

	std::unordered_map<GLenum, std::string>
		OpenGLShader::Parse(const std::string& source) {
		std::unordered_map<GLenum, std::string> shaders;

		const char* typeToken = "#type";
		size_t pos = source.find(typeToken, 0);

		while (pos != std::string::npos) {
			size_t eol = source.find("\n", pos);
			size_t begin = pos + strlen(typeToken) + 1;

			std::string type = source.substr(begin, eol - begin);

			GLenum glType =
				type == "vertex" ? GL_VERTEX_SHADER :
				type == "fragment" ? GL_FRAGMENT_SHADER :
				type == "compute" ? GL_COMPUTE_SHADER :
				0;

			size_t nextLine = source.find("\n", eol) + 1;
			pos = source.find(typeToken, nextLine);

			shaders[glType] = source.substr(
				nextLine,
				pos == std::string::npos ? std::string::npos : pos - nextLine
			);
		}

		return shaders;
	}

	void OpenGLShader::Compile(
		const std::unordered_map<GLenum, std::string>& shaders) {
		GLuint program = glCreateProgram();

		std::vector<GLuint> shaderIDs;

		for (auto& [type, src] : shaders) {
			GLuint shader = glCreateShader(type);
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
			shaderIDs.push_back(shader);
		}

		glLinkProgram(program);

		m_RendererID = program;

		for (auto id : shaderIDs)
			glDeleteShader(id);
	}

	void OpenGLShader::Reflect() {
		GLint count;
		glGetProgramiv(m_RendererID, GL_ACTIVE_UNIFORMS, &count);

		for (int i = 0; i < count; i++) {
			char name[256];
			GLsizei length;
			GLint size;
			GLenum type;

			glGetActiveUniform(m_RendererID, i, 256, &length, &size, &type, name);
			GLint location = glGetUniformLocation(m_RendererID, name);

			UniformInfo info;
			info.Location = location;
			info.Type = type;
			info.Size = size;
			info.IsTexture = (type == GL_SAMPLER_2D || type == GL_SAMPLER_CUBE);

			m_UniformLocationCache[name] = info;

			std::cout << "[Shader] Uniform: " << name
				<< " | Location: " << location
				<< " | Type: " << type
				<< " | Size: " << size
				<< " | Texture: " << info.IsTexture
				<< std::endl;
		}
	}

	void OpenGLShader::Bind() const {
		glUseProgram(m_RendererID);
	}

	void OpenGLShader::Unbind() const {
		glUseProgram(0);
	}
}