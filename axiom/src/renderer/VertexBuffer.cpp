#include "axiom/renderer/VertexBuffer.h"
#include "axiom/renderer/RendererAPI.h"
#include "axiom/platform/OpenGL/OpenGLVertexBuffer.h"
namespace axiom {

	std::shared_ptr<VertexBuffer> VertexBuffer::Create(void* vertices, uint32_t size) {
		switch (RendererAPI::GetAPI()) {
		case RendererAPIType::OpenGL:
			return  std::make_shared<OpenGLVertexBuffer>(vertices, size);
		}
		return nullptr;
	}
}