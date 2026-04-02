#include "axiom/platform/OpenGL/OpenGLVertexArray.h"
#include "axiom/platform/OpenGL/OpenGLVertexBuffer.h"
#include <glad/glad.h>

OpenGLVertexArray::OpenGLVertexArray() {
    glCreateVertexArrays(1, &m_ID);
}

OpenGLVertexArray::~OpenGLVertexArray() {
    glDeleteVertexArrays(1, &m_ID);
}

void OpenGLVertexArray::Bind() const {
    glBindVertexArray(m_ID);
}

void OpenGLVertexArray::Unbind() const {
    glBindVertexArray(0);
}

void OpenGLVertexArray::AddVertexBuffer(const std::shared_ptr<VertexBuffer>& vertexBuffer) {
    Bind();
    vertexBuffer->Bind();

    const auto& layout = vertexBuffer->GetLayout();

    for (const auto& element : layout.GetElements()) {
        glEnableVertexAttribArray(m_VertexBufferIndex);

        glVertexAttribPointer(
            m_VertexBufferIndex,
            GetComponentCount(element.Type),
            GL_FLOAT,
            element.Normalized ? GL_TRUE : GL_FALSE,
            layout.GetStride(),
            (const void*)(uintptr_t)element.Offset
        );

        m_VertexBufferIndex++;
    }

    m_VertexBuffers.push_back(vertexBuffer);
}