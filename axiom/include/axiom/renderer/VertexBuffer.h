#pragma once
#include <cstdint>

class VertexBuffer {
public:
    virtual ~VertexBuffer() = default;

    virtual void Bind() const = 0;
    virtual void Unbind() const = 0;

    static VertexBuffer* Create(float* vertices, uint32_t size);
};