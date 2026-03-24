#pragma once

#include <cstdint>
#include <memory>

#include "axiom/renderer/VertexLayout.h"

namespace axiom {

    // ===================== VERTEX BUFFER =====================

    class VertexBuffer {
    public:

        VertexBuffer(const void* data, uint32_t size);
        ~VertexBuffer();

        void Bind() const;

        void SetLayout(const VertexLayout& layout) {
            m_Layout = layout;
        }
        const VertexLayout& GetLayout() const {
            return m_Layout;
        }

    private:

        uint32_t m_ID;
        VertexLayout m_Layout;
    };

    // ===================== INDEX BUFFER =====================

    class IndexBuffer {
    public:

        IndexBuffer(const uint32_t* indices, uint32_t count);
        ~IndexBuffer();

        void Bind() const;

        uint32_t GetCount() const {
            return m_Count;
        }

    private:

        uint32_t m_ID;
        uint32_t m_Count;
    };

}