#pragma once
#include <cstdint>

namespace axiom {

	class IndexBuffer {
	public:
		IndexBuffer(const uint32_t* indices, uint32_t count);
		~IndexBuffer();

		void bind() const;
		uint32_t getCount() const {
			return m_Count;
		}

		uint32_t getRendererID() const {
			return m_RendererID;
		}

	private:
		uint32_t m_RendererID;
		uint32_t m_Count;
	};

}
