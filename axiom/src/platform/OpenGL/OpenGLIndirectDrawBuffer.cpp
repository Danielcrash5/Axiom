#include "axiom/platform/OpenGL/OpenGLIndirectDrawBuffer.h"
#include <glad/glad.h>

namespace axiom {
	OpenGLIndirectDrawBuffer::OpenGLIndirectDrawBuffer() {
		glGenBuffers(1, &m_BufferID);
	}

	OpenGLIndirectDrawBuffer::~OpenGLIndirectDrawBuffer() {
		if (m_BufferID)
			glDeleteBuffers(1, &m_BufferID);
	}

	void OpenGLIndirectDrawBuffer::SetCommands(const std::vector<DrawElementsIndirectCommand>& commands) {
		m_Commands = commands;
		glBindBuffer(GL_DRAW_INDIRECT_BUFFER, m_BufferID);
		glBufferData(GL_DRAW_INDIRECT_BUFFER, m_Commands.size() * sizeof(DrawElementsIndirectCommand), m_Commands.data(), GL_DYNAMIC_DRAW);
		glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);
	}

	void OpenGLIndirectDrawBuffer::Bind() const {
		glBindBuffer(GL_DRAW_INDIRECT_BUFFER, m_BufferID);
	}

	void OpenGLIndirectDrawBuffer::Unbind() const {
		glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);
	}

	const DrawElementsIndirectCommand& OpenGLIndirectDrawBuffer::GetCommand(uint32_t index) const {
		return m_Commands[index];
	}

	uint32_t OpenGLIndirectDrawBuffer::GetCount() const {
		return static_cast<uint32_t>(m_Commands.size());
	}
}
