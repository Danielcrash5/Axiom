#pragma once
#include <vector>
#include <cstdint>
#include <glad/glad.h>

namespace axiom {

	struct VertexBufferElement {
		uint32_t type;
		uint32_t count;
		bool normalized;

		static uint32_t getSizeOfType(uint32_t type) {
			switch (type) {
			case GL_FLOAT: return 4;
			case GL_UNSIGNED_INT: return 4;
			case GL_UNSIGNED_BYTE: return 1;
			}
			return 0;
		}
	};

	class VertexBufferLayout {
	public:
		VertexBufferLayout() = default;

		template<typename T>
		void push(uint32_t count);

		inline const std::vector<VertexBufferElement>& getElements() const {
			return m_Elements;
		}
		inline uint32_t getStride() const {
			return m_Stride;
		}

	private:
		std::vector<VertexBufferElement> m_Elements;
		uint32_t m_Stride = 0;
	};

	template<>
	inline void VertexBufferLayout::push<float>(uint32_t count) {
		m_Elements.push_back({ GL_FLOAT, count, GL_FALSE });
		m_Stride += count * VertexBufferElement::getSizeOfType(GL_FLOAT);
	}

	template<>
	inline void VertexBufferLayout::push<uint32_t>(uint32_t count) {
		m_Elements.push_back({ GL_UNSIGNED_INT, count, GL_FALSE });
		m_Stride += count * VertexBufferElement::getSizeOfType(GL_UNSIGNED_INT);
	}

	template<>
	inline void VertexBufferLayout::push<unsigned char>(uint32_t count) {
		m_Elements.push_back({ GL_UNSIGNED_BYTE, count, GL_TRUE });
		m_Stride += count * VertexBufferElement::getSizeOfType(GL_UNSIGNED_BYTE);
	}

}