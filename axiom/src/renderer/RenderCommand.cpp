#include "axiom/renderer/RenderCommand.h"
#include "axiom/platform/OpenGL/OpenGLRendererAPI.h"
namespace axiom {
	void RenderCommand::Init() {
		s_RendererAPI = new OpenGLRendererAPI();
		s_RendererAPI->Init();
	}

	RendererAPI* RenderCommand::s_RendererAPI = nullptr;
}