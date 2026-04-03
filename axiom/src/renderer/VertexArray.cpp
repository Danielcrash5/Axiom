#include "axiom/renderer/VertexArray.h"
#include "axiom/platform/OpenGL/OpenGLVertexArray.h"
#include "axiom/renderer/RendererAPI.h"
namespace axiom {
	std::shared_ptr<VertexArray> VertexArray::Create() {
		if (RendererAPI::GetAPI() == RendererAPIType::OpenGL) {
			return std::make_shared<OpenGLVertexArray>();
		}
	}
}