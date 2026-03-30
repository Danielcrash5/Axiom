#pragma once
#include <cstdint>

namespace axiom {

    class VertexBuffer;
    class IndexBuffer;
    class VertexBufferLayout;

    class VertexArray {
    public:
        VertexArray();
        ~VertexArray();

        void bind() const;

        void setVertexBuffer(const VertexBuffer& vb, const VertexBufferLayout& layout);
        void setIndexBuffer(const IndexBuffer& ib);

    private:
        uint32_t m_RendererID;
    };

}