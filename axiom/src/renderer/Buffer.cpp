#include "axiom/renderer/Buffer.h"

#include <glad/glad.h>

namespace axiom {

    // ===================== VERTEX BUFFER =====================

    VertexBuffer::VertexBuffer(const void* data, uint32_t size) {
        glCreateBuffers(1, &m_ID);
        glBindBuffer(GL_ARRAY_BUFFER, m_ID);
        glBufferData(GL_ARRAY_BUFFER, size, data, GL_DYNAMIC_DRAW);
    }

    VertexBuffer::~VertexBuffer() {
        glDeleteBuffers(1, &m_ID);
    }

    void VertexBuffer::Bind() const {
        glBindBuffer(GL_ARRAY_BUFFER, m_ID);
    }

    // ===================== INDEX BUFFER =====================

    IndexBuffer::IndexBuffer(const uint32_t* indices, uint32_t count)
        : m_Count(count) {
        glCreateBuffers(1, &m_ID);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ID);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(uint32_t), indices, GL_STATIC_DRAW);
    }

    IndexBuffer::~IndexBuffer() {
        glDeleteBuffers(1, &m_ID);
    }

    void IndexBuffer::Bind() const {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ID);
    }

}