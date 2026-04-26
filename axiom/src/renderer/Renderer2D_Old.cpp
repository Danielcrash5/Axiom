#include "axiom/renderer/Renderer2D.h"

#include "axiom/assets/AssetManager.h"
#include "axiom/renderer/IndexBuffer.h"
#include "axiom/renderer/Material.h"
#include "axiom/renderer/RenderCommand.h"
#include "axiom/renderer/Renderer.h"
#include "axiom/renderer/Shader.h"
#include "axiom/renderer/Sprite.h"
#include "axiom/renderer/Texture2D.h"
#include "axiom/renderer/VertexArray.h"
#include "axiom/renderer/VertexBuffer.h"

#include <cmath>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

namespace axiom {

Renderer2D::RendererData* Renderer2D::s_Data = nullptr;

namespace {

constexpr float kEpsilon = 0.0001f;

glm::vec2 GetDefaultQuadUV(int index) {
    static const glm::vec2 kUVs[4] = {
        { 0.0f, 0.0f },
        { 1.0f, 0.0f },
        { 1.0f, 1.0f },
        { 0.0f, 1.0f }
    };

    return kUVs[index];
}

void Configure2DMaterial(const std::shared_ptr<Material>& material) {
    if (!material)
        return;

    auto& state = material->GetRenderState();
    state.DepthTest = true;
    state.DepthFunction = DepthFunc::LessEqual;
    state.DepthWrite = true;
    state.Blending = true;
    state.BlendSrc = BlendFactor::SrcAlpha;
    state.BlendDst = BlendFactor::OneMinusSrcAlpha;
    state.CullFace = false;
}

void DrawIndexedNow(
    const std::shared_ptr<VertexArray>& vertexArray,
    const std::shared_ptr<Material>& material,
    uint32_t indexCount,
    const glm::mat4& transform
) {
    if (!vertexArray || !material || indexCount == 0)
        return;

    material->Set("u_ViewProjection", Renderer::GetViewProjection());
    material->Set("u_Transform", transform);
    material->Bind();
    RenderCommand::DrawIndexed(vertexArray, indexCount);
}

} // namespace

void Renderer2D::Init() {
    s_Data = new RendererData();

    auto whiteTexture = Texture2D::Create(
        1,
        1,
        false,
        TextureWrap::ClampToEdge,
        TextureWrap::ClampToEdge,
        TextureFilter::Linear,
        TextureFilter::Linear
    );
    uint32_t whitePixel = 0xffffffff;
    whiteTexture->SetData(&whitePixel, 1, 1, TextureFormat::RGBA8);
    s_Data->TextureSlots[0] = whiteTexture;

    s_Data->QuadVAO = VertexArray::Create();
    s_Data->QuadVBO = VertexBuffer::Create(4 * sizeof(QuadVertex), BufferUsage::Dynamic);
    s_Data->QuadVBO->SetLayout(BufferLayout(sizeof(QuadVertex), {
        { ShaderDataType::Vec3, "a_Position", static_cast<uint32_t>(offsetof(QuadVertex, Position)), false },
        { ShaderDataType::Vec4, "a_Color", static_cast<uint32_t>(offsetof(QuadVertex, Color)), false },
        { ShaderDataType::Vec2, "a_TexCoord", static_cast<uint32_t>(offsetof(QuadVertex, TexCoord)), false },
        { ShaderDataType::Float, "a_TexIndex", static_cast<uint32_t>(offsetof(QuadVertex, TexIndex)), false },
        { ShaderDataType::Float, "a_TilingFactor", static_cast<uint32_t>(offsetof(QuadVertex, TilingFactor)), false }
    }));
    s_Data->QuadVAO->AddVertexBuffer(s_Data->QuadVBO);

    uint32_t quadIndices[6] = { 0, 1, 2, 2, 3, 0 };
    s_Data->QuadIBO = IndexBuffer::Create(quadIndices, 6);
    s_Data->QuadVAO->SetIndexBuffer(s_Data->QuadIBO);

    s_Data->CircleVAO = VertexArray::Create();
    s_Data->CircleVBO = VertexBuffer::Create(4 * sizeof(CircleVertex), BufferUsage::Dynamic);
    s_Data->CircleVBO->SetLayout(BufferLayout(sizeof(CircleVertex), {
        { ShaderDataType::Vec3, "a_Position", static_cast<uint32_t>(offsetof(CircleVertex, Position)), false },
        { ShaderDataType::Vec3, "a_LocalPosition", static_cast<uint32_t>(offsetof(CircleVertex, LocalPosition)), false },
        { ShaderDataType::Vec4, "a_Color", static_cast<uint32_t>(offsetof(CircleVertex, Color)), false },
        { ShaderDataType::Float, "a_Thickness", static_cast<uint32_t>(offsetof(CircleVertex, Thickness)), false }
    }));
    s_Data->CircleVAO->AddVertexBuffer(s_Data->CircleVBO);
    s_Data->CircleVAO->SetIndexBuffer(s_Data->QuadIBO);

    s_Data->SkinnedVAO = VertexArray::Create();
    s_Data->SkinnedVBO = VertexBuffer::Create(RendererData::MaxVertices * sizeof(SkinnedVertex2D), BufferUsage::Dynamic);
    s_Data->SkinnedVBO->SetLayout(BufferLayout(sizeof(SkinnedVertex2D), {
        { ShaderDataType::Vec3, "a_Position", static_cast<uint32_t>(offsetof(SkinnedVertex2D, Position)), false },
        { ShaderDataType::Vec4, "a_Color", static_cast<uint32_t>(offsetof(SkinnedVertex2D, Color)), false },
        { ShaderDataType::Vec2, "a_TexCoord", static_cast<uint32_t>(offsetof(SkinnedVertex2D, TexCoord)), false },
        { ShaderDataType::Vec4, "a_BoneIndices", static_cast<uint32_t>(offsetof(SkinnedVertex2D, BoneIndices)), false },
        { ShaderDataType::Vec4, "a_BoneWeights", static_cast<uint32_t>(offsetof(SkinnedVertex2D, BoneWeights)), false }
    }));
    s_Data->SkinnedVAO->AddVertexBuffer(s_Data->SkinnedVBO);

    auto quadShader = AssetManager::Get<Shader>("engine://shaders/Renderer2D/Quad.glsl");
    auto circleShader = AssetManager::Get<Shader>("engine://shaders/Renderer2D/Circle.glsl");
    auto skinnedShader = AssetManager::Get<Shader>("engine://shaders/Renderer2D/Skinned.glsl");

    s_Data->QuadMaterial = std::make_shared<Material>(quadShader);
    s_Data->CircleMaterial = std::make_shared<Material>(circleShader);
    s_Data->SkinnedMaterial = std::make_shared<Material>(skinnedShader);

    Configure2DMaterial(s_Data->QuadMaterial);
    Configure2DMaterial(s_Data->CircleMaterial);
    Configure2DMaterial(s_Data->SkinnedMaterial);

    quadShader->Bind();
    for (uint32_t i = 0; i < RendererData::MaxTextureSlots; ++i)
        quadShader->SetUniform1i("u_Textures[" + std::to_string(i) + "]", static_cast<int>(i));

    s_Data->QuadVertexPositions[0] = { -0.5f, -0.5f, 0.0f };
    s_Data->QuadVertexPositions[1] = { 0.5f, -0.5f, 0.0f };
    s_Data->QuadVertexPositions[2] = { 0.5f, 0.5f, 0.0f };
    s_Data->QuadVertexPositions[3] = { -0.5f, 0.5f, 0.0f };

    s_Data->QuadBufferBase = new QuadVertex[4];
    s_Data->CircleBufferBase = new CircleVertex[4];
}

void Renderer2D::Shutdown() {
    delete[] s_Data->QuadBufferBase;
    delete[] s_Data->CircleBufferBase;
    delete s_Data;
    s_Data = nullptr;
}

void Renderer2D::BeginScene() {
}

void Renderer2D::EndScene() {
}

void Renderer2D::StartBatch() {
}

void Renderer2D::FlushAll() {
}

void Renderer2D::FlushQuads() {
}

void Renderer2D::FlushCircles() {
}

void Renderer2D::EnsureQuadCapacity(uint32_t) {
}

void Renderer2D::EnsureCircleCapacity(uint32_t) {
}

float Renderer2D::AcquireTextureSlot(const std::shared_ptr<Texture2D>&) {
    return 0.0f;
}

void Renderer2D::Configure2DMaterial(const std::shared_ptr<Material>& material) {
    ::axiom::Configure2DMaterial(material);
}

void Renderer2D::DrawQuad(const glm::vec2& pos, const glm::vec2& size, const glm::vec4& color, float z) {
    DrawQuad(glm::vec3(pos, z), size, color);
}

void Renderer2D::DrawQuad(const glm::vec3& pos, const glm::vec2& size, const glm::vec4& color) {
    const glm::mat4 transform =
        glm::translate(glm::mat4(1.0f), pos) *
        glm::scale(glm::mat4(1.0f), glm::vec3(size, 1.0f));

    DrawQuad(transform, color);
}

void Renderer2D::DrawQuad(const glm::mat4& transform, const glm::vec4& color) {
    for (int i = 0; i < 4; ++i) {
        s_Data->QuadBufferBase[i].Position = s_Data->QuadVertexPositions[i];
        s_Data->QuadBufferBase[i].Color = color;
        s_Data->QuadBufferBase[i].TexCoord = GetDefaultQuadUV(i);
        s_Data->QuadBufferBase[i].TexIndex = 0.0f;
        s_Data->QuadBufferBase[i].TilingFactor = 1.0f;
    }

    s_Data->TextureSlots[0]->Bind(0);
    s_Data->QuadVBO->SetData(s_Data->QuadBufferBase, 4 * sizeof(QuadVertex));
    DrawIndexedNow(s_Data->QuadVAO, s_Data->QuadMaterial, 6, transform);
}

void Renderer2D::DrawQuad(
    const glm::vec2& pos,
    const glm::vec2& size,
    const std::shared_ptr<Texture2D>& texture,
    float tiling,
    const glm::vec4& tint,
    float z
) {
    DrawQuad(glm::vec3(pos, z), size, texture, tiling, tint);
}

void Renderer2D::DrawQuad(
    const glm::vec3& pos,
    const glm::vec2& size,
    const std::shared_ptr<Texture2D>& texture,
    float tiling,
    const glm::vec4& tint
) {
    const glm::mat4 transform =
        glm::translate(glm::mat4(1.0f), pos) *
        glm::scale(glm::mat4(1.0f), glm::vec3(size, 1.0f));

    DrawQuad(transform, texture, tiling, tint);
}

void Renderer2D::DrawQuad(
    const glm::mat4& transform,
    const std::shared_ptr<Texture2D>& texture,
    float tiling,
    const glm::vec4& tint
) {
    for (int i = 0; i < 4; ++i) {
        s_Data->QuadBufferBase[i].Position = s_Data->QuadVertexPositions[i];
        s_Data->QuadBufferBase[i].Color = tint;
        s_Data->QuadBufferBase[i].TexCoord = GetDefaultQuadUV(i);
        s_Data->QuadBufferBase[i].TexIndex = texture ? 1.0f : 0.0f;
        s_Data->QuadBufferBase[i].TilingFactor = tiling;
    }

    s_Data->TextureSlots[0]->Bind(0);
    if (texture)
        texture->Bind(1);

    s_Data->QuadVBO->SetData(s_Data->QuadBufferBase, 4 * sizeof(QuadVertex));
    DrawIndexedNow(s_Data->QuadVAO, s_Data->QuadMaterial, 6, transform);
}

void Renderer2D::DrawQuad(
    const glm::mat4& transform,
    const std::shared_ptr<Texture2D>& texture,
    const std::shared_ptr<Material>& materialOverride,
    float tiling,
    const glm::vec4& tint
) {
    if (!materialOverride) {
        DrawQuad(transform, texture, tiling, tint);
        return;
    }

    for (int i = 0; i < 4; ++i) {
        s_Data->QuadBufferBase[i].Position = s_Data->QuadVertexPositions[i];
        s_Data->QuadBufferBase[i].Color = tint;
        s_Data->QuadBufferBase[i].TexCoord = GetDefaultQuadUV(i);
        s_Data->QuadBufferBase[i].TexIndex = 0.0f;
        s_Data->QuadBufferBase[i].TilingFactor = tiling;
    }

    s_Data->QuadVBO->SetData(s_Data->QuadBufferBase, 4 * sizeof(QuadVertex));

    Configure2DMaterial(materialOverride);
    if (texture && materialOverride->GetShader() && materialOverride->GetShader()->HasUniform("u_Texture"))
        materialOverride->SetTexture("u_Texture", texture);

    DrawIndexedNow(s_Data->QuadVAO, materialOverride, 6, transform);
}

void Renderer2D::DrawSprite(const glm::mat4& transform, const Sprite& sprite, const glm::vec4& tint) {
    for (int i = 0; i < 4; ++i) {
        s_Data->QuadBufferBase[i].Position = s_Data->QuadVertexPositions[i];
        s_Data->QuadBufferBase[i].Color = tint;
        s_Data->QuadBufferBase[i].TexCoord = glm::vec2(
            (i == 1 || i == 2) ? sprite.UVMax.x : sprite.UVMin.x,
            (i >= 2) ? sprite.UVMax.y : sprite.UVMin.y
        );
        s_Data->QuadBufferBase[i].TexIndex = sprite.Texture ? 1.0f : 0.0f;
        s_Data->QuadBufferBase[i].TilingFactor = 1.0f;
    }

    s_Data->TextureSlots[0]->Bind(0);
    if (sprite.Texture)
        sprite.Texture->Bind(1);

    s_Data->QuadVBO->SetData(s_Data->QuadBufferBase, 4 * sizeof(QuadVertex));
    DrawIndexedNow(s_Data->QuadVAO, s_Data->QuadMaterial, 6, transform);
}

void Renderer2D::DrawSprite(
    const glm::mat4& transform,
    const Sprite& sprite,
    const glm::vec4& tint,
    const std::shared_ptr<Material>& materialOverride
) {
    if (!materialOverride) {
        DrawSprite(transform, sprite, tint);
        return;
    }

    for (int i = 0; i < 4; ++i) {
        s_Data->QuadBufferBase[i].Position = s_Data->QuadVertexPositions[i];
        s_Data->QuadBufferBase[i].Color = tint;
        s_Data->QuadBufferBase[i].TexCoord = glm::vec2(
            (i == 1 || i == 2) ? sprite.UVMax.x : sprite.UVMin.x,
            (i >= 2) ? sprite.UVMax.y : sprite.UVMin.y
        );
        s_Data->QuadBufferBase[i].TexIndex = 0.0f;
        s_Data->QuadBufferBase[i].TilingFactor = 1.0f;
    }

    s_Data->QuadVBO->SetData(s_Data->QuadBufferBase, 4 * sizeof(QuadVertex));

    Configure2DMaterial(materialOverride);
    if (sprite.Texture && materialOverride->GetShader() && materialOverride->GetShader()->HasUniform("u_Texture"))
        materialOverride->SetTexture("u_Texture", sprite.Texture);

    DrawIndexedNow(s_Data->QuadVAO, materialOverride, 6, transform);
}

void Renderer2D::DrawSkinned(
    const glm::mat4& transform,
    const SkinnedMesh2D& mesh,
    const SkeletonPose2D& pose,
    const glm::vec4& tint,
    const std::shared_ptr<Material>& materialOverride
) {
    if (mesh.Vertices.empty() || mesh.Indices.empty())
        return;

    std::vector<SkinnedVertex2D> vertices = mesh.Vertices;
    for (auto& vertex : vertices)
        vertex.Color *= tint;

    s_Data->SkinnedVBO->SetData(vertices.data(), static_cast<uint32_t>(vertices.size() * sizeof(SkinnedVertex2D)));
    s_Data->SkinnedIBO = IndexBuffer::Create(const_cast<uint32_t*>(mesh.Indices.data()), static_cast<uint32_t>(mesh.Indices.size()));
    s_Data->SkinnedVAO->SetIndexBuffer(s_Data->SkinnedIBO);

    auto material = materialOverride ? materialOverride : s_Data->SkinnedMaterial;
    Configure2DMaterial(material);

    auto shader = material->GetShader();
    shader->Bind();

    const uint32_t boneCount = pose.BoneCount < MaxBones ? pose.BoneCount : MaxBones;
    for (uint32_t i = 0; i < boneCount; ++i)
        shader->SetUniformMat4("u_BoneTransforms[" + std::to_string(i) + "]", pose.BoneTransforms[i]);

    if (mesh.Texture && shader->HasUniform("u_Texture")) {
        mesh.Texture->Bind(0);
        shader->SetUniform1i("u_Texture", 0);
    }

    DrawIndexedNow(s_Data->SkinnedVAO, material, s_Data->SkinnedIBO->GetCount(), transform);
}

void Renderer2D::DrawLine(const glm::vec3& p0, const glm::vec3& p1, const glm::vec4& color) {
    DrawLine(p0, p1, color, 1.5f);
}

void Renderer2D::DrawLine(const glm::vec3& p0, const glm::vec3& p1, const glm::vec4& color, float thickness) {
    DrawLine(p0, p1, color, thickness, LineCap::Butt);
}

void Renderer2D::DrawLine(const glm::vec3& p0, const glm::vec3& p1, const glm::vec4& color, float thickness, LineCap cap) {
    if (thickness <= 0.0f)
        return;

    const glm::vec2 delta = glm::vec2(p1) - glm::vec2(p0);
    const float length = glm::length(delta);
    if (length <= kEpsilon) {
        DrawCircle(p0, thickness * 0.5f, 0.0f, color);
        return;
    }

    glm::vec3 start = p0;
    glm::vec3 end = p1;
    const glm::vec2 direction = delta / length;
    const float halfThickness = thickness * 0.5f;

    if (cap == LineCap::Square) {
        start -= glm::vec3(direction * halfThickness, 0.0f);
        end += glm::vec3(direction * halfThickness, 0.0f);
    }

    const glm::vec2 adjustedDelta = glm::vec2(end) - glm::vec2(start);
    const float adjustedLength = glm::length(adjustedDelta);
    const glm::vec3 center = (start + end) * 0.5f;
    const float angle = std::atan2(adjustedDelta.y, adjustedDelta.x);

    const glm::mat4 transform =
        glm::translate(glm::mat4(1.0f), center) *
        glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0.0f, 0.0f, 1.0f)) *
        glm::scale(glm::mat4(1.0f), glm::vec3(adjustedLength, thickness, 1.0f));

    DrawQuad(transform, color);

    if (cap == LineCap::Round) {
        DrawCircle(p0, halfThickness, 0.0f, color);
        DrawCircle(p1, halfThickness, 0.0f, color);
    }
}

void Renderer2D::DrawLineStrip(
    const std::vector<glm::vec3>& points,
    const glm::vec4& color,
    float thickness,
    bool closed,
    LineCap cap,
    LineJoin join,
    float miterLimit
) {
    if (points.size() < 2 || thickness <= 0.0f)
        return;

    const size_t pointCount = points.size();
    const size_t segmentCount = closed ? pointCount : pointCount - 1;
    const LineCap segmentCap = closed ? LineCap::Butt : cap;

    for (size_t i = 0; i < segmentCount; ++i) {
        const glm::vec3& a = points[i];
        const glm::vec3& b = points[(i + 1) % pointCount];
        DrawLine(a, b, color, thickness, segmentCap);

        if (i + 1 < pointCount || closed) {
            const glm::vec3& c = points[(i + 2) % pointCount];
            glm::vec2 v1 = glm::normalize(glm::vec2(b) - glm::vec2(a));
            glm::vec2 v2 = glm::normalize(glm::vec2(c) - glm::vec2(b));
            const float dot = glm::clamp(glm::dot(v1, v2), -1.0f, 1.0f);
            const float angle = std::acos(dot);

            switch (join) {
            case LineJoin::Round:
                DrawCircle(b, thickness * 0.5f, 0.0f, color);
                break;
            case LineJoin::Bevel:
                break;
            case LineJoin::Miter:
                if (angle < glm::pi<float>() * 0.5f && miterLimit > 0.0f)
                    DrawCircle(b, thickness * 0.35f, 0.0f, color);
                break;
            }
        }
    }
}

void Renderer2D::DrawRect(const glm::vec3& pos, const glm::vec2& size, const glm::vec4& color, float z, float thickness) {
    const glm::vec3 p0 = pos + glm::vec3(0.0f, 0.0f, z);
    const glm::vec3 p1 = pos + glm::vec3(size.x, 0.0f, z);
    const glm::vec3 p2 = pos + glm::vec3(size.x, size.y, z);
    const glm::vec3 p3 = pos + glm::vec3(0.0f, size.y, z);

    DrawLine(p0, p1, color, thickness, LineCap::Butt);
    DrawLine(p1, p2, color, thickness, LineCap::Butt);
    DrawLine(p2, p3, color, thickness, LineCap::Butt);
    DrawLine(p3, p0, color, thickness, LineCap::Butt);
}

void Renderer2D::DrawRect(const glm::mat4& transform, const glm::vec4& color, float thickness) {
    glm::vec3 points[4];
    for (int i = 0; i < 4; ++i)
        points[i] = transform * glm::vec4(s_Data->QuadVertexPositions[i], 1.0f);

    DrawLine(points[0], points[1], color, thickness, LineCap::Butt);
    DrawLine(points[1], points[2], color, thickness, LineCap::Butt);
    DrawLine(points[2], points[3], color, thickness, LineCap::Butt);
    DrawLine(points[3], points[0], color, thickness, LineCap::Butt);
}

void Renderer2D::DrawCircle(const glm::vec2& pos, float radius, float thickness, const glm::vec4& color, float z) {
    DrawCircle(glm::vec3(pos, z), radius, thickness, color);
}

void Renderer2D::DrawCircle(const glm::vec3& pos, float radius, float thickness, const glm::vec4& color) {
    const glm::mat4 transform =
        glm::translate(glm::mat4(1.0f), pos) *
        glm::scale(glm::mat4(1.0f), glm::vec3(radius * 2.0f, radius * 2.0f, 1.0f));

    DrawCircle(transform, thickness, color);
}

void Renderer2D::DrawCircle(const glm::mat4& transform, float thickness, const glm::vec4& color) {
    for (int i = 0; i < 4; ++i) {
        s_Data->CircleBufferBase[i].Position = s_Data->QuadVertexPositions[i];
        s_Data->CircleBufferBase[i].LocalPosition = glm::vec3(glm::vec2(s_Data->QuadVertexPositions[i]) * 2.0f, 0.0f);
        s_Data->CircleBufferBase[i].Color = color;
        s_Data->CircleBufferBase[i].Thickness = thickness;
    }

    s_Data->CircleVBO->SetData(s_Data->CircleBufferBase, 4 * sizeof(CircleVertex));
    DrawIndexedNow(s_Data->CircleVAO, s_Data->CircleMaterial, 6, transform);
}

} // namespace axiom
