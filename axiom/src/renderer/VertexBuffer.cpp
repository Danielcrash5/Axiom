#include "axiom/renderer/VertexBuffer.h"
#include "axiom/renderer/RendererAPI.h"
#include "axiom/platform/OpenGL/OpenGLVertexBuffer.h"

VertexBuffer* VertexBuffer::Create(float* vertices, uint32_t size) {
    switch (RendererAPI::GetAPI()) {
    case RendererAPIType::OpenGL:
        return new OpenGLVertexBuffer(vertices, size);
    }
    return nullptr;
}