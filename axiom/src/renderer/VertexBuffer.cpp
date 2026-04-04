#include "axiom/renderer/VertexBuffer.h"
#include "axiom/platform/OpenGL/OpenGLVertexBuffer.h"

namespace axiom {

    std::shared_ptr<VertexBuffer> VertexBuffer::Create(uint32_t size, BufferUsage usage) {
        return std::make_shared<OpenGLVertexBuffer>(size, usage);
    }

    std::shared_ptr<VertexBuffer> VertexBuffer::Create(void* vertices, uint32_t size) {
        return std::make_shared<OpenGLVertexBuffer>(vertices, size);
    }

}