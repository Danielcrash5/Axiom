// OpenGLVertexArray.h
#pragma once
#include "axiom/renderer/VertexArray.h"

namespace axiom {

	class OpenGLVertexArray : public VertexArray {
	public:
		OpenGLVertexArray();
		~OpenGLVertexArray();

		void Bind() const override;
		void Unbind() const override;

		void AddVertexBuffer(const std::shared_ptr<VertexBuffer>& vb) override;
		void SetIndexBuffer(const std::shared_ptr<IndexBuffer>& ib) override;

		const std::vector<std::shared_ptr<VertexBuffer>>& GetVertexBuffers() const override {
			return m_VertexBuffers;
		}

	private:
		uint32_t m_ID;
		uint32_t m_VertexBufferIndex = 0;

		std::vector<std::shared_ptr<VertexBuffer>> m_VertexBuffers;
		std::shared_ptr<IndexBuffer> m_IndexBuffer;
	};
}