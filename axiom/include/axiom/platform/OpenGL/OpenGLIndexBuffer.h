#pragma once
#include "axiom/renderer/IndexBuffer.h"
namespace axiom {
	class OpenGLIndexBuffer : public IndexBuffer {
	public:
		OpenGLIndexBuffer(uint32_t* indices, uint32_t count);
		~OpenGLIndexBuffer();

		void Bind() const override;
		uint32_t GetCount() const override;

	private:
		uint32_t m_RendererID;
		uint32_t m_Count;
	};
}