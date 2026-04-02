#include "ShaderDataType.h"
#include <vector>
#include <string>

struct BufferElement {
    std::string Name;
    ShaderDataType Type;
    bool Normalized;
    uint32_t Size;
    uint32_t Offset;

    BufferElement(ShaderDataType type, const std::string& name, bool normalized = true)
        : Name(name), Type(type),Normalized(normalized), Size(GetSize(type)), Offset(0) {
    }
};

class BufferLayout {
public:
    BufferLayout() = default;
    BufferLayout(const std::initializer_list<BufferElement>& elements)
        : m_Elements(elements) {
        uint32_t offset = 0;
        for (auto& e : m_Elements) {
            e.Offset = offset;
            offset += e.Size;
        }
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