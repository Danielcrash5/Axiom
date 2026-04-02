#include "axiom/renderer/Shader.h"
#include "axiom/renderer/RendererAPI.h"
#include "axiom/platform/OpenGL/OpenGLShader.h"

std::shared_ptr<Shader> Shader::Create(const std::string& path) {
	if (RendererAPI::GetAPI() == RendererAPIType::OpenGL) {
		return std::make_shared<OpenGLShader>(path);
	}
}
