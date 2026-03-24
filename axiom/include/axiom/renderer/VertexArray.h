#pragma once

#include <memory>
#include <vector>

#include "axiom/renderer/Buffer.h"

namespace axiom {

    class VertexArray {
    public:

        VertexArray();
        ~VertexArray();

        void Bind() const;

        void AddVertexBuffer(const std::shared_ptr<VertexBuffer>& vb);
        void SetIndexBuffer(const std::shared_ptr<IndexBuffer>& ib);

        const std::shared_ptr<IndexBuffer>& GetIndexBuffer() const {
            return m_IndexBuffer;
        }

    private:

        uint32_t m_ID;
        uint32_t m_AttribIndex = 0;

        std::vector<std::shared_ptr<VertexBuffer>> m_VertexBuffers;
        std::shared_ptr<IndexBuffer> m_IndexBuffer;
    };

}