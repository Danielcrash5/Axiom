#include "axiom/renderer/Shader.h"
#include "axiom/renderer/RendererAPI.h"
#include "axiom/platform/OpenGL/OpenGLShader.h"

namespace axiom {
	std::shared_ptr<Shader> Shader::Create(
		const std::string& path,
		const std::unordered_set<std::string>& defines) {
		return std::make_shared<OpenGLShader>(path, defines);
	}

    std::shared_ptr<Shader> Shader::CreateFromMemory(
        const std::string& source,
        const std::unordered_set<std::string>& defines
    ) {
        switch (RendererAPI::GetAPI()) {
        case RendererAPIType::OpenGL:
            return std::make_shared<OpenGLShader>(source, defines, /*fromMemory=*/true);
        }
        return nullptr;
    }

}