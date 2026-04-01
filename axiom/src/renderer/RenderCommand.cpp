#include "axiom/renderer/RenderCommand.h"
#include "axiom/platform/OpenGL/OpenGLRendererAPI.h"

RendererAPI* RenderCommand::s_RendererAPI = new OpenGLRendererAPI();