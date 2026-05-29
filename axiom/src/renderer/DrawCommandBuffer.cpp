#include "axiom/renderer/DrawCommandBuffer.h"

#include <algorithm>

namespace axiom {

namespace {

float ExtractSortZ(const glm::mat4& transform) {
    return transform[3].z;
}

uintptr_t GetTextureSortKey(const Sprite& sprite) {
    return reinterpret_cast<uintptr_t>(sprite.Texture.get());
}

} // namespace

void DrawCommandBuffer2D::Clear() {
    m_Commands.clear();
    m_NextSequence = 0;
}

void DrawCommandBuffer2D::SubmitQuad(const glm::mat4& transform, const glm::vec4& color) {
    DrawCommand2D command;
    command.Type = DrawCommand2DType::Quad;
    command.Transform = transform;
    command.Color = color;
    command.SortZ = ExtractSortZ(transform);
    Submit(command);
}

void DrawCommandBuffer2D::SubmitSprite(const glm::mat4& transform, const Sprite& sprite, const glm::vec4& color) {
    DrawCommand2D command;
    command.Type = DrawCommand2DType::Sprite;
    command.Transform = transform;
    command.Color = color;
    command.SpriteData = sprite;
    command.SortZ = ExtractSortZ(transform);
    command.MaterialSortKey = GetTextureSortKey(sprite);
    Submit(command);
}

void DrawCommandBuffer2D::SubmitCircle(const glm::mat4& transform, float thickness, const glm::vec4& color) {
    DrawCommand2D command;
    command.Type = DrawCommand2DType::Circle;
    command.Transform = transform;
    command.Color = color;
    command.Thickness = thickness;
    command.SortZ = ExtractSortZ(transform);
    Submit(command);
}

void DrawCommandBuffer2D::Submit(DrawCommand2D command) {
    command.Sequence = m_NextSequence++;
    m_Commands.push_back(command);
}

void DrawCommandBuffer2D::Sort() {
    std::stable_sort(
        m_Commands.begin(),
        m_Commands.end(),
        [](const DrawCommand2D& a, const DrawCommand2D& b) {
            if (a.SortZ != b.SortZ)
                return a.SortZ < b.SortZ;

            if (a.MaterialSortKey != b.MaterialSortKey)
                return a.MaterialSortKey < b.MaterialSortKey;

            if (a.Type != b.Type)
                return static_cast<int>(a.Type) < static_cast<int>(b.Type);

            return a.Sequence < b.Sequence;
        }
    );
}

void DrawCommandBuffer2D::Execute() const {
    for (const DrawCommand2D& command : m_Commands) {
        switch (command.Type) {
        case DrawCommand2DType::Quad:
            Renderer2D::DrawQuad(command.Transform, command.Color);
            break;
        case DrawCommand2DType::Sprite:
            Renderer2D::DrawSprite(command.Transform, command.SpriteData, command.Color);
            break;
        case DrawCommand2DType::Circle:
            Renderer2D::DrawCircle(command.Transform, command.Thickness, command.Color);
            break;
        }
    }
}

} // namespace axiom
