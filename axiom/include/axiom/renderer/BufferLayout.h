#include "ShaderDataType.h"
#include <vector>
#include <string>

struct BufferElement {
    std::string Name;
    ShaderDataType Type;
    bool Normalized;
    uint32_t Size;
    uint32_t Offset;
    bool HasExplicitOffset;

    BufferElement(ShaderDataType type, const std::string& name, bool normalized = true)
        : Name(name), Type(type), Normalized(normalized), Size(GetSize(type)), Offset(0), HasExplicitOffset(false) {
    }

    BufferElement(ShaderDataType type, const std::string& name, uint32_t offset, bool normalized)
        : Name(name), Type(type), Normalized(normalized), Size(GetSize(type)), Offset(offset), HasExplicitOffset(true) {
    }
};

class BufferLayout {
public:
    BufferLayout() = default;
    BufferLayout(const std::initializer_list<BufferElement>& elements)
        : m_Elements(elements) {
        CalculateOffsetsAndStride();
    }

    BufferLayout(uint32_t stride, const std::initializer_list<BufferElement>& elements)
        : m_Elements(elements), m_Stride(stride) {
        CalculateOffsetsAndStride();
    }

    void CalculateOffsetsAndStride() {
        uint32_t offset = 0;
        for (auto& e : m_Elements) {
            if (!e.HasExplicitOffset)
                e.Offset = offset;

            offset = e.Offset + e.Size;
        }

        if (m_Stride == 0)
            m_Stride = offset;
    }

    uint32_t GetStride() const {
        return m_Stride;
    }
    const auto& GetElements() const {
        return m_Elements;
    }

private:
    std::vector<BufferElement> m_Elements;
    uint32_t m_Stride = 0;
};
