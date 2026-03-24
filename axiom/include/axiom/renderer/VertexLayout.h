#pragma once

#include <vector>
#include <string>
#include <cstdint>

namespace axiom {

    // ===================== DATA TYPES =====================

    enum class ShaderDataType {
        Float,
        Vec2,
        Vec3,
        Vec4,

        Mat3,
        Mat4,

        Int,
        IVec2,
        IVec3,
        IVec4,

        Bool
    };

    // ===================== HELPERS =====================

    static uint32_t ShaderDataTypeSize(ShaderDataType type) {
        switch (type) {
        case ShaderDataType::Float: return 4;
        case ShaderDataType::Vec2:  return 4 * 2;
        case ShaderDataType::Vec3:  return 4 * 3;
        case ShaderDataType::Vec4:  return 4 * 4;

        case ShaderDataType::Mat3:  return 4 * 3 * 3;
        case ShaderDataType::Mat4:  return 4 * 4 * 4;

        case ShaderDataType::Int:   return 4;
        case ShaderDataType::IVec2: return 4 * 2;
        case ShaderDataType::IVec3: return 4 * 3;
        case ShaderDataType::IVec4: return 4 * 4;

        case ShaderDataType::Bool:  return 1;
        }
        return 0;
    }

    static uint32_t ShaderDataTypeComponentCount(ShaderDataType type) {
        switch (type) {
        case ShaderDataType::Float: return 1;
        case ShaderDataType::Vec2:  return 2;
        case ShaderDataType::Vec3:  return 3;
        case ShaderDataType::Vec4:  return 4;

        case ShaderDataType::Mat3:  return 3 * 3;
        case ShaderDataType::Mat4:  return 4 * 4;

        case ShaderDataType::Int:   return 1;
        case ShaderDataType::IVec2: return 2;
        case ShaderDataType::IVec3: return 3;
        case ShaderDataType::IVec4: return 4;

        case ShaderDataType::Bool:  return 1;
        }
        return 0;
    }

    // ===================== ELEMENT =====================

    struct BufferElement {
        std::string Name;
        ShaderDataType Type;

        uint32_t Size;
        uint32_t Offset;

        bool Normalized;

        BufferElement() = default;

        BufferElement(ShaderDataType type, const std::string& name, bool normalized = false)
            : Name(name), Type(type),
            Size(ShaderDataTypeSize(type)),
            Offset(0),
            Normalized(normalized) {
        }
    };

    // ===================== LAYOUT =====================

    class VertexLayout {
    public:

        VertexLayout() = default;

        VertexLayout(const std::initializer_list<BufferElement>& elements)
            : m_Elements(elements) {
            Calculate();
        }

        const std::vector<BufferElement>& GetElements() const {
            return m_Elements;
        }
        uint32_t GetStride() const {
            return m_Stride;
        }

    private:

        void Calculate() {
            uint32_t offset = 0;
            m_Stride = 0;

            for (auto& e : m_Elements) {
                e.Offset = offset;
                offset += e.Size;
                m_Stride += e.Size;
            }
        }

    private:

        std::vector<BufferElement> m_Elements;
        uint32_t m_Stride = 0;
    };

}