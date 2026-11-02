#include <axiom/renderer/Renderer2D.h>
#include <axiom/renderer/Renderer.h>

#include <cmath>
#include <glm/gtc/matrix_transform.hpp>

namespace axiom::renderer {

Renderer* Renderer2D::s_renderer = nullptr;

Renderer2D::SceneData& Renderer2D::scene() {
    static SceneData data;
    return data;
}

void Renderer2D::setActiveRenderer(Renderer* renderer) {
    s_renderer = renderer;
}

Renderer* Renderer2D::activeRenderer() {
    return s_renderer;
}

void Renderer2D::BeginScene(const glm::mat4& viewProjection) {
    auto& data = scene();
    data.viewProjection = viewProjection;
    data.active = true;
    data.nextZIndex = 0;
    clearQueuedItems();
}

void Renderer2D::beginScene(const glm::mat4& viewProjection) {
    BeginScene(viewProjection);
}

void Renderer2D::EndScene() {
    Flush();
    scene().active = false;
}

void Renderer2D::endScene() {
    EndScene();
}

void Renderer2D::Flush() {
    auto& data = scene();
    if (!data.active && data.queuedItems.empty()) {
        return;
    }

    if (s_renderer) {
        for (auto& item : data.queuedItems) {
            s_renderer->submitItem(item);
        }
    }

    data.queuedItems.clear();
    data.nextZIndex = 0;
}

void Renderer2D::flush() {
    Flush();
}

void Renderer2D::pushItem(RenderItem item) {
    auto& data = scene();
    if (!data.active) {
        data.active = true;
    }
    item.zIndex = data.nextZIndex++;
    item.layer = 1u;
    data.queuedItems.push_back(std::move(item));
}

void Renderer2D::SubmitQuad(const glm::mat4& transform, const glm::vec4& color,
                            float tilingFactor) {
    RenderItem item;
    item.material = std::make_shared<Material>();
    item.material->shaderId = static_cast<ShaderID>(1);
    item.transform = transform;
    item.vertexBuffer = rhi::BufferHandle{1, 1};
    item.indexBuffer = rhi::BufferHandle{2, 1};
    item.indexCount = 6;
    item.material->params["baseColor"] = ResourceBinding{};
    item.material->params["baseColor"].binding = 0;
    item.material->params["baseColor"].kind = ResourceBinding::Kind::Direct;
    item.material->params["baseColor"].buffer = item.vertexBuffer;
    item.material->state = rhi::PipelineState{};
    item.material->state.blend = rhi::BlendMode::AlphaBlend;
    item.depth = 0.0f;
    item.transform = transform;
    item.material->params["tilingFactor"] = ResourceBinding{};
    item.material->params["tilingFactor"].binding = 1;
    item.material->params["tilingFactor"].kind = ResourceBinding::Kind::Direct;
    (void)color;
    (void)tilingFactor;
    pushItem(item);
}

void Renderer2D::SubmitCircle(const glm::mat4& transform, float thickness,
                              const glm::vec4& color) {
    RenderItem item;
    item.material = std::make_shared<Material>();
    item.material->shaderId = static_cast<ShaderID>(2);
    item.transform = transform;
    item.vertexBuffer = rhi::BufferHandle{3, 1};
    item.indexBuffer = rhi::BufferHandle{4, 1};
    item.indexCount = 6;
    item.depth = 0.0f;
    (void)thickness;
    (void)color;
    pushItem(item);
}

void Renderer2D::SubmitLine(const glm::vec3& start, const glm::vec3& end,
                           const glm::vec4& color) {
    const glm::vec3 center = (start + end) * 0.5f;
    const glm::vec3 delta = end - start;
    const float length = glm::length(delta);
    const float thickness = 0.05f;

    glm::mat4 transform = glm::translate(glm::mat4(1.0f), center);
    if (length > 0.0001f) {
        const float angle = std::atan2(delta.y, delta.x);
        transform = glm::translate(glm::mat4(1.0f), center) *
                    glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0.0f, 0.0f, 1.0f)) *
                    glm::scale(glm::mat4(1.0f), glm::vec3(length, thickness, 1.0f));
    }

    RenderItem item;
    item.material = std::make_shared<Material>();
    item.material->shaderId = static_cast<ShaderID>(3);
    item.transform = transform;
    item.vertexBuffer = rhi::BufferHandle{5, 1};
    item.indexBuffer = rhi::BufferHandle{6, 1};
    item.indexCount = 2;
    item.depth = 0.0f;
    (void)color;
    pushItem(item);
}

void Renderer2D::SubmitRect(const glm::vec3& position, const glm::vec2& size,
                           const glm::vec4& color) {
    const glm::mat4 transform =
        glm::translate(glm::mat4(1.0f), position) *
        glm::scale(glm::mat4(1.0f), glm::vec3(size.x, size.y, 1.0f));
    SubmitQuad(transform, color);
}

void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size,
                         const glm::vec4& color) {
    SubmitRect(glm::vec3(position, 0.0f), size, color);
}

void Renderer2D::DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size,
                                float rotationRadians,
                                const glm::vec4& color) {
    glm::mat4 transform = glm::translate(glm::mat4(1.0f),
                                       glm::vec3(position, 0.0f)) *
                          glm::rotate(glm::mat4(1.0f), rotationRadians,
                                      glm::vec3(0.0f, 0.0f, 1.0f)) *
                          glm::scale(glm::mat4(1.0f),
                                    glm::vec3(size.x, size.y, 1.0f));
    SubmitQuad(transform, color);
}

void Renderer2D::DrawCircle(const glm::vec2& position, float radius,
                           const glm::vec4& color,
                           float thickness) {
    glm::mat4 transform = glm::translate(glm::mat4(1.0f),
                                       glm::vec3(position, 0.0f)) *
                          glm::scale(glm::mat4(1.0f),
                                    glm::vec3(radius, radius, 1.0f));
    SubmitCircle(transform, thickness, color);
}

void Renderer2D::DrawLine(const glm::vec2& start, const glm::vec2& end,
                         const glm::vec4& color) {
    SubmitLine(glm::vec3(start, 0.0f), glm::vec3(end, 0.0f), color);
}

std::span<const RenderItem> Renderer2D::queuedItems() {
    return std::span<const RenderItem>(scene().queuedItems);
}

size_t Renderer2D::queuedItemCount() {
    return scene().queuedItems.size();
}

void Renderer2D::clearQueuedItems() {
    scene().queuedItems.clear();
    scene().nextZIndex = 0;
}

void Renderer2D::submitQuad(const glm::mat4& transform, const glm::vec4& color,
                           float tilingFactor) {
    SubmitQuad(transform, color, tilingFactor);
}

void Renderer2D::submitCircle(const glm::mat4& transform, float thickness,
                             const glm::vec4& color) {
    SubmitCircle(transform, thickness, color);
}

void Renderer2D::submitLine(const glm::vec3& start, const glm::vec3& end,
                           const glm::vec4& color) {
    SubmitLine(start, end, color);
}

} // namespace axiom::renderer
