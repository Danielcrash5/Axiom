#pragma once

#include "axiom/renderer/Renderer2D.h"
#include "axiom/renderer/Sprite.h"

#include <glm/glm.hpp>

#include <cstdint>
#include <vector>

namespace axiom {

    enum class DrawCommand2DType {
        Quad,
        Sprite,
        Circle
    };

    struct DrawCommand2D {
        DrawCommand2DType Type = DrawCommand2DType::Quad;
        glm::mat4 Transform { 1.0f };
        glm::vec4 Color { 1.0f };
        Sprite SpriteData {};
        float Thickness = 1.0f;
        float SortZ = 0.0f;
        uintptr_t MaterialSortKey = 0;
        uint64_t Sequence = 0;
    };

    class DrawCommandBuffer2D {
    public:
        void Clear();

        void SubmitQuad(const glm::mat4& transform, const glm::vec4& color);
        void SubmitSprite(const glm::mat4& transform, const Sprite& sprite, const glm::vec4& color);
        void SubmitCircle(const glm::mat4& transform, float thickness, const glm::vec4& color);

        void Sort();
        void Execute() const;

        bool Empty() const {
            return m_Commands.empty();
        }

        size_t Size() const {
            return m_Commands.size();
        }

    private:
        void Submit(DrawCommand2D command);

    private:
        std::vector<DrawCommand2D> m_Commands;
        uint64_t m_NextSequence = 0;
    };

} // namespace axiom
