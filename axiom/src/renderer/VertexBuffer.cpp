#include "axiom/renderer/VertexBuffer.h"
#include <glad/glad.h>

namespace axiom {
	VertexBuffer::VertexBuffer(uint32_t size) {
		glCreateBuffers(1, &m_RendererID);
		glNamedBufferData(m_RendererID, size, nullptr, GL_DYNAMIC_DRAW);
	}

	VertexBuffer::VertexBuffer(const void* data, uint32_t size) {
		glCreateBuffers(1, &m_RendererID);
		glNamedBufferData(m_RendererID, size, data, GL_STATIC_DRAW);
	}

	VertexBuffer::~VertexBuffer() {
		glDeleteBuffers(1, &m_RendererID);
	}

	void VertexBuffer::setData(const void* data, uint32_t size) {
		glNamedBufferSubData(m_RendererID, 0, size, data);
	}

	void VertexBuffer::bind() const {
		glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
	}
}