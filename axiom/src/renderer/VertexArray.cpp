#include "axiom/renderer/VertexArray.h"
#include "axiom/core/Logger.h"

#include <glad/glad.h>

namespace axiom {

    // ===================== HELPERS =====================

    static GLenum ShaderDataTypeToOpenGLBaseType(ShaderDataType type) {
        switch (type) {
        case ShaderDataType::Float:
        case ShaderDataType::Vec2:
        case ShaderDataType::Vec3:
        case ShaderDataType::Vec4:
        case ShaderDataType::Mat3:
        case ShaderDataType::Mat4:
            return GL_FLOAT;

        case ShaderDataType::Int:
        case ShaderDataType::IVec2:
        case ShaderDataType::IVec3:
        case ShaderDataType::IVec4:
            return GL_INT;

        case ShaderDataType::Bool:
            return GL_INT;
        }

        AXIOM_ASSERT(false, "Unknown ShaderDataType!");
        return GL_FLOAT; // fallback
    }

    // ===================== VAO =====================

    VertexArray::VertexArray() {
        glGenVertexArrays(1, &m_ID);
    }

    VertexArray::~VertexArray() {
        glDeleteVertexArrays(1, &m_ID);
    }

    void VertexArray::Bind() const {
        glBindVertexArray(m_ID);
    }

    // ===================== ADD VB =====================

    void VertexArray::AddVertexBuffer(const std::shared_ptr<VertexBuffer>& vb) {
        Bind();
        vb->Bind();

        const auto layout = vb->GetLayout();

		for (const auto& element : layout.GetElements()) {
			glEnableVertexAttribArray(m_AttribIndex);

			glVertexAttribPointer(
				m_AttribIndex,
				ShaderDataTypeComponentCount(element.Type),
				ShaderDataTypeToOpenGLBaseType(element.Type),
				element.Normalized ? GL_TRUE : GL_FALSE,
				layout.GetStride(),
				(const void*)(uintptr_t)element.Offset
			);

			m_AttribIndex++;
		}

        m_VertexBuffers.push_back(vb);
    }

    // ===================== INDEX =====================

    void VertexArray::SetIndexBuffer(const std::shared_ptr<IndexBuffer>& ib) {
        Bind();
        ib->Bind();

        m_IndexBuffer = ib;
    }

}