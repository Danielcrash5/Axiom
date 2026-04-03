#pragma once
#include "axiom/renderer/VertexBuffer.h"

namespace axiom {

	class OpenGLVertexBuffer : public VertexBuffer {
	public:
		OpenGLVertexBuffer(void* vertices, uint32_t size);
		~OpenGLVertexBuffer();

		void Bind() const override;
		void Unbind() const override;

	private:
		uint32_t m_RendererID;
	};
}