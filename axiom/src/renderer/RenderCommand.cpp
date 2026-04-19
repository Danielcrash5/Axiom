#include "axiom/renderer/RenderCommand.h"
#include "axiom/platform/OpenGL/OpenGLRendererAPI.h"

namespace axiom {
	RendererAPI* RenderCommand::s_RendererAPI = new OpenGLRendererAPI();
}