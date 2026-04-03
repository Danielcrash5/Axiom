#include "axiom/renderer/Shader.h"
#include "axiom/platform/OpenGL/OpenGLShader.h"

namespace axiom {
	std::shared_ptr<Shader> Shader::Create(
		const std::string& path,
		const std::unordered_set<std::string>& defines) {
		return std::make_shared<OpenGLShader>(path, defines);
	}
}