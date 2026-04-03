#pragma once
#include <cstdint>
#include <memory>

#include "BufferLayout.h"

namespace axiom {
	class VertexBuffer {
	public:
		virtual ~VertexBuffer() = default;

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		static std::shared_ptr<VertexBuffer> Create(void* vertices, uint32_t size);

		void SetLayout(const BufferLayout& layout) {
			m_Layout = layout;
		}
		const BufferLayout& GetLayout() const {
			return m_Layout;
		}
	private:
		BufferLayout m_Layout;
	};
}