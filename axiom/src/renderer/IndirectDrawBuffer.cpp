#include "axiom/renderer/IndirectDrawBuffer.h"
#include "axiom/platform/OpenGL/OpenGLIndirectDrawBuffer.h"

namespace axiom {
	std::shared_ptr<IndirectDrawBuffer> IndirectDrawBuffer::Create() {
		// Später: API switch
		return std::make_shared<OpenGLIndirectDrawBuffer>();
	}
}
