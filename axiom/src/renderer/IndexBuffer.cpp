#include "axiom/renderer/IndexBuffer.h"
#include "axiom/renderer/RendererAPI.h"
#include "axiom/platform/OpenGL/OpenGLIndexBuffer.h"

IndexBuffer* IndexBuffer::Create(uint32_t* indices, uint32_t count) {
    switch (RendererAPI::GetAPI()) {
    case RendererAPIType::OpenGL:
        return new OpenGLIndexBuffer(indices, count);
    }
    return nullptr;
}