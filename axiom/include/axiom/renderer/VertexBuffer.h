#pragma once
#include <cstdint>

namespace axiom {
	class VertexBuffer {
	public:
		VertexBuffer(uint32_t size);
		VertexBuffer(const void* data, uint32_t size);
		~VertexBuffer();

		void setData(const void* data, uint32_t size);

		void bind() const;

		uint32_t getRendererID() const {
			return m_RendererID;
		}

	private:
		uint32_t m_RendererID;
	};
}