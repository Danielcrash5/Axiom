#pragma once
#include "axiom/renderer/VertexBuffer.h"

namespace axiom {

    class OpenGLVertexBuffer : public VertexBuffer {
    public:
        // Dynamischer Buffer
        OpenGLVertexBuffer(uint32_t size, BufferUsage usage);

        // Statischer Buffer
        OpenGLVertexBuffer(void* vertices, uint32_t size);

        virtual ~OpenGLVertexBuffer();

        void Bind() const override;
        void Unbind() const override;

        void SetData(const void* data, uint32_t size) override;

    private:
        uint32_t m_RendererID = 0;
        BufferUsage m_Usage;
    };

}