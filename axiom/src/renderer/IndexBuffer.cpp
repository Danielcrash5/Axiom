#include "axiom/renderer/IndexBuffer.h"
#include "axiom/renderer/RendererAPI.h"
#include "axiom/platform/OpenGL/OpenGLIndexBuffer.h"

namespace axiom {
	std::shared_ptr<IndexBuffer> IndexBuffer::Create(uint32_t* indices, uint32_t count) {
		switch (RendererAPI::GetAPI()) {
		case RendererAPIType::OpenGL:
			return std::make_shared<OpenGLIndexBuffer>(indices, count);
		}
		return nullptr;
	}
}