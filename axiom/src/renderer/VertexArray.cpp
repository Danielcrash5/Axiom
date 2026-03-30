#include "axiom/renderer/VertexArray.h"
#include "axiom/renderer/VertexBuffer.h"
#include "axiom/renderer/IndexBuffer.h"
#include <glad/glad.h>

namespace axiom {

	VertexArray::VertexArray() {
		glCreateVertexArrays(1, &m_RendererID);
	}

	VertexArray::~VertexArray() {
		glDeleteVertexArrays(1, &m_RendererID);
	}

	void VertexArray::bind() const {
		glBindVertexArray(m_RendererID);
	}

    void VertexArray::setVertexBuffer(const VertexBuffer& vb, const VertexBufferLayout& layout) {
        uint32_t bindingIndex = 0;

        glVertexArrayVertexBuffer(m_RendererID, bindingIndex, vb.getRendererID(), 0, layout.getStride());

        const auto& elements = layout.getElements();
        uint32_t offset = 0;

        for (uint32_t i = 0; i < elements.size(); i++) {
            const auto& element = elements[i];

            glEnableVertexArrayAttrib(m_RendererID, i);
            glVertexArrayAttribFormat(
                m_RendererID,
                i,
                element.count,
                element.type,
                element.normalized,
                offset
            );

            glVertexArrayAttribBinding(m_RendererID, i, bindingIndex);

            offset += element.count * VertexBufferElement::getSizeOfType(element.type);
        }
    }

	void VertexArray::setIndexBuffer(const IndexBuffer& ib) {
		glVertexArrayElementBuffer(m_RendererID, ib.getRendererID()); // ✔ korrekt
	}

}