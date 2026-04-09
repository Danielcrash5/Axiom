#include "axiom/renderer/Renderer2D.h"
#include "axiom/renderer/Renderer.h"
#include "axiom/renderer/RenderCommand.h"
#include "axiom/assets/AssetManager.h"

#include <algorithm>
#include <cstddef>
#include <cmath>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string>

namespace axiom {

    Renderer2D::RendererData* Renderer2D::s_Data = nullptr;

    namespace {
        constexpr float kEpsilon = 0.0001f;

        glm::vec2 Perpendicular(const glm::vec2& v) {
            return glm::vec2(-v.y, v.x);
        }

        float Cross(const glm::vec2& a, const glm::vec2& b) {
            return a.x * b.y - a.y * b.x;
        }

        std::shared_ptr<Material> CreateImmediateColorMaterial() {
            static const char* source = R"(#type vertex
#version 460 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Color;
layout(location = 2) in vec2 a_TexCoord;
layout(location = 3) in float a_TexIndex;
layout(location = 4) in float a_TilingFactor;

uniform mat4 u_ViewProjection;
uniform mat4 u_Transform;

out vec4 v_Color;

void main()
{
    v_Color = a_Color;
    gl_Position = u_ViewProjection * u_Transform * vec4(a_Position, 1.0);
}

#type fragment
#version 460 core

layout(location = 0) out vec4 FragColor;

in vec4 v_Color;

void main()
{
    FragColor = v_Color;
}
)";

            return std::make_shared<Material>(Shader::CreateFromMemory(source));
        }

        std::shared_ptr<Material> CreateImmediateQuadMaterial() {
            static const char* source = R"(#type vertex
#version 460 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Color;
layout(location = 2) in vec2 a_TexCoord;
layout(location = 3) in float a_TexIndex;
layout(location = 4) in float a_TilingFactor;

uniform mat4 u_ViewProjection;
uniform mat4 u_Transform;

out vec4 v_Color;
out vec2 v_TexCoord;
out float v_TilingFactor;

void main()
{
    v_Color = a_Color;
    v_TexCoord = a_TexCoord;
    v_TilingFactor = a_TilingFactor;
    gl_Position = u_ViewProjection * u_Transform * vec4(a_Position, 1.0);
}

#type fragment
#version 460 core

layout(location = 0) out vec4 FragColor;

in vec4 v_Color;
in vec2 v_TexCoord;
in float v_TilingFactor;

uniform sampler2D u_Texture;

void main()
{
    FragColor = texture(u_Texture, v_TexCoord * v_TilingFactor) * v_Color;
}
)";

            return std::make_shared<Material>(Shader::CreateFromMemory(source));
        }
    }

    void Renderer2D::Init() {
        s_Data = new RendererData();

        auto whiteTexture = Texture2D::Create(1, 1, false,
                                              TextureWrap::ClampToEdge, TextureWrap::ClampToEdge,
                                              TextureFilter::Linear, TextureFilter::Linear);
        uint32_t white = 0xffffffff;
        whiteTexture->SetData(&white, 1, 1, TextureFormat::RGBA8);

        s_Data->TextureSlots[0] = whiteTexture;
        s_Data->TextureSlotIndex = 1;

        s_Data->QuadVAO = VertexArray::Create();
        s_Data->QuadVBO = VertexBuffer::Create(
            RendererData::MaxVertices * sizeof(QuadVertex),
            BufferUsage::Dynamic
        );
        s_Data->QuadVBO->SetLayout(BufferLayout(sizeof(QuadVertex), {
            { ShaderDataType::Vec3, "a_Position", static_cast<uint32_t>(offsetof(QuadVertex, Position)), false },
            { ShaderDataType::Vec4, "a_Color", static_cast<uint32_t>(offsetof(QuadVertex, Color)), false },
            { ShaderDataType::Vec2, "a_TexCoord", static_cast<uint32_t>(offsetof(QuadVertex, TexCoord)), false },
            { ShaderDataType::Float, "a_TexIndex", static_cast<uint32_t>(offsetof(QuadVertex, TexIndex)), false },
            { ShaderDataType::Float, "a_TilingFactor", static_cast<uint32_t>(offsetof(QuadVertex, TilingFactor)), false }
        }));
        s_Data->QuadVAO->AddVertexBuffer(s_Data->QuadVBO);

        uint32_t* quadIndices = new uint32_t[RendererData::MaxIndices];
        uint32_t quadOffset = 0;
        for (uint32_t i = 0; i < RendererData::MaxIndices; i += 6) {
            quadIndices[i + 0] = quadOffset + 0;
            quadIndices[i + 1] = quadOffset + 1;
            quadIndices[i + 2] = quadOffset + 2;
            quadIndices[i + 3] = quadOffset + 2;
            quadIndices[i + 4] = quadOffset + 3;
            quadIndices[i + 5] = quadOffset + 0;
            quadOffset += 4;
        }
        s_Data->QuadIBO = IndexBuffer::Create(quadIndices, RendererData::MaxIndices);
        s_Data->QuadVAO->SetIndexBuffer(s_Data->QuadIBO);
        delete[] quadIndices;
        s_Data->QuadBufferBase = new QuadVertex[RendererData::MaxVertices];

        s_Data->ImmediateQuadVAO = VertexArray::Create();
        s_Data->ImmediateQuadVBO = VertexBuffer::Create(
            6 * sizeof(QuadVertex),
            BufferUsage::Dynamic
        );
        s_Data->ImmediateQuadVBO->SetLayout(BufferLayout(sizeof(QuadVertex), {
            { ShaderDataType::Vec3, "a_Position", static_cast<uint32_t>(offsetof(QuadVertex, Position)), false },
            { ShaderDataType::Vec4, "a_Color", static_cast<uint32_t>(offsetof(QuadVertex, Color)), false },
            { ShaderDataType::Vec2, "a_TexCoord", static_cast<uint32_t>(offsetof(QuadVertex, TexCoord)), false },
            { ShaderDataType::Float, "a_TexIndex", static_cast<uint32_t>(offsetof(QuadVertex, TexIndex)), false },
            { ShaderDataType::Float, "a_TilingFactor", static_cast<uint32_t>(offsetof(QuadVertex, TilingFactor)), false }
        }));
        s_Data->ImmediateQuadVAO->AddVertexBuffer(s_Data->ImmediateQuadVBO);

        uint32_t immediateQuadIndices[6] = { 0, 1, 2, 3, 4, 5 };
        s_Data->ImmediateQuadIBO = IndexBuffer::Create(immediateQuadIndices, 6);
        s_Data->ImmediateQuadVAO->SetIndexBuffer(s_Data->ImmediateQuadIBO);

        s_Data->LineVAO = VertexArray::Create();
        s_Data->LineVBO = VertexBuffer::Create(
            RendererData::MaxVertices * sizeof(LineVertex),
            BufferUsage::Dynamic
        );
        s_Data->LineVBO->SetLayout(BufferLayout(sizeof(LineVertex), {
            { ShaderDataType::Vec3, "a_Position", static_cast<uint32_t>(offsetof(LineVertex, Position)), false },
            { ShaderDataType::Vec4, "a_Color", static_cast<uint32_t>(offsetof(LineVertex, Color)), false }
        }));
        s_Data->LineVAO->AddVertexBuffer(s_Data->LineVBO);

        uint32_t* lineIndices = new uint32_t[RendererData::MaxVertices];
        for (uint32_t i = 0; i < RendererData::MaxVertices; i++) {
            lineIndices[i] = i;
        }
        s_Data->LineIBO = IndexBuffer::Create(lineIndices, RendererData::MaxVertices);
        s_Data->LineVAO->SetIndexBuffer(s_Data->LineIBO);
        delete[] lineIndices;
        s_Data->LineBufferBase = new LineVertex[RendererData::MaxVertices];

        s_Data->CircleVAO = VertexArray::Create();
        s_Data->CircleVBO = VertexBuffer::Create(
            RendererData::MaxVertices * sizeof(CircleVertex),
            BufferUsage::Dynamic
        );
        s_Data->CircleVBO->SetLayout(BufferLayout(sizeof(CircleVertex), {
            { ShaderDataType::Vec3, "a_Position", static_cast<uint32_t>(offsetof(CircleVertex, Position)), false },
            { ShaderDataType::Vec3, "a_LocalPosition", static_cast<uint32_t>(offsetof(CircleVertex, LocalPosition)), false },
            { ShaderDataType::Vec4, "a_Color", static_cast<uint32_t>(offsetof(CircleVertex, Color)), false },
            { ShaderDataType::Float, "a_Thickness", static_cast<uint32_t>(offsetof(CircleVertex, Thickness)), false }
        }));
        s_Data->CircleVAO->AddVertexBuffer(s_Data->CircleVBO);
        s_Data->CircleVAO->SetIndexBuffer(s_Data->QuadIBO);
        s_Data->CircleBufferBase = new CircleVertex[RendererData::MaxVertices];

        s_Data->SkinnedVAO = VertexArray::Create();
        s_Data->SkinnedVBO = VertexBuffer::Create(
            RendererData::MaxVertices * sizeof(SkinnedVertex2D),
            BufferUsage::Dynamic
        );
        s_Data->SkinnedVBO->SetLayout(BufferLayout(sizeof(SkinnedVertex2D), {
            { ShaderDataType::Vec3, "a_Position", static_cast<uint32_t>(offsetof(SkinnedVertex2D, Position)), false },
            { ShaderDataType::Vec4, "a_Color", static_cast<uint32_t>(offsetof(SkinnedVertex2D, Color)), false },
            { ShaderDataType::Vec2, "a_TexCoord", static_cast<uint32_t>(offsetof(SkinnedVertex2D, TexCoord)), false },
            { ShaderDataType::Vec4, "a_BoneIndices", static_cast<uint32_t>(offsetof(SkinnedVertex2D, BoneIndices)), false },
            { ShaderDataType::Vec4, "a_BoneWeights", static_cast<uint32_t>(offsetof(SkinnedVertex2D, BoneWeights)), false }
        }));
        s_Data->SkinnedVAO->AddVertexBuffer(s_Data->SkinnedVBO);

        auto quadShader = AssetManager::Get<Shader>("engine://shaders/Renderer2D/Quad.glsl");
        auto lineShader = AssetManager::Get<Shader>("engine://shaders/Renderer2D/Line.glsl");
        auto circleShader = AssetManager::Get<Shader>("engine://shaders/Renderer2D/Circle.glsl");
        auto skinnedShader = AssetManager::Get<Shader>("engine://shaders/Renderer2D/Skinned.glsl");

        s_Data->QuadMaterial = std::make_shared<Material>(quadShader);
        s_Data->ImmediateColorMaterial = CreateImmediateColorMaterial();
        s_Data->ImmediateQuadMaterial = CreateImmediateQuadMaterial();
        s_Data->LineMaterial = std::make_shared<Material>(lineShader);
        s_Data->CircleMaterial = std::make_shared<Material>(circleShader);
        s_Data->SkinnedMaterial = std::make_shared<Material>(skinnedShader);

        Configure2DMaterial(s_Data->QuadMaterial);
        Configure2DMaterial(s_Data->ImmediateColorMaterial);
        Configure2DMaterial(s_Data->ImmediateQuadMaterial);
        Configure2DMaterial(s_Data->LineMaterial);
        Configure2DMaterial(s_Data->CircleMaterial);
        Configure2DMaterial(s_Data->SkinnedMaterial);

        quadShader->Bind();
        for (uint32_t i = 0; i < RendererData::MaxTextureSlots; i++) {
            quadShader->SetUniform1i("u_Textures[" + std::to_string(i) + "]", static_cast<int>(i));
        }

        s_Data->QuadVertexPositions[0] = { -0.5f, -0.5f, 0.0f, 1.0f };
        s_Data->QuadVertexPositions[1] = { 0.5f, -0.5f, 0.0f, 1.0f };
        s_Data->QuadVertexPositions[2] = { 0.5f, 0.5f, 0.0f, 1.0f };
        s_Data->QuadVertexPositions[3] = { -0.5f, 0.5f, 0.0f, 1.0f };

        StartBatch();
    }

    void Renderer2D::Shutdown() {
        delete[] s_Data->QuadBufferBase;
        delete[] s_Data->LineBufferBase;
        delete[] s_Data->CircleBufferBase;
        delete s_Data;
        s_Data = nullptr;
    }

    void Renderer2D::BeginScene() {
        StartBatch();
    }

    void Renderer2D::EndScene() {
        FlushAll();
    }

    void Renderer2D::StartBatch() {
        s_Data->QuadBufferPtr = s_Data->QuadBufferBase;
        s_Data->LineBufferPtr = s_Data->LineBufferBase;
        s_Data->CircleBufferPtr = s_Data->CircleBufferBase;
        s_Data->TextureSlotIndex = 1;
    }

    void Renderer2D::FlushAll() {
        FlushQuads();
        FlushLines();
        FlushCircles();
        StartBatch();
    }

    void Renderer2D::EnsureQuadCapacity(uint32_t quadCount) {
        const uint32_t usedVertices = static_cast<uint32_t>(s_Data->QuadBufferPtr - s_Data->QuadBufferBase);
        if (usedVertices + quadCount * 4 > RendererData::MaxVertices) {
            FlushAll();
        }
    }

    void Renderer2D::EnsureLineCapacity(uint32_t vertexCount) {
        const uint32_t usedVertices = static_cast<uint32_t>(s_Data->LineBufferPtr - s_Data->LineBufferBase);
        if (usedVertices + vertexCount > RendererData::MaxVertices) {
            FlushAll();
        }
    }

    void Renderer2D::EnsureCircleCapacity(uint32_t circleCount) {
        const uint32_t usedVertices = static_cast<uint32_t>(s_Data->CircleBufferPtr - s_Data->CircleBufferBase);
        if (usedVertices + circleCount * 4 > RendererData::MaxVertices) {
            FlushAll();
        }
    }

    float Renderer2D::AcquireTextureSlot(const std::shared_ptr<Texture2D>& texture) {
        if (!texture) {
            return 0.0f;
        }

        for (uint32_t i = 1; i < s_Data->TextureSlotIndex; i++) {
            if (s_Data->TextureSlots[i].get() == texture.get()) {
                return static_cast<float>(i);
            }
        }

        if (s_Data->TextureSlotIndex >= RendererData::MaxTextureSlots) {
            FlushAll();
        }

        const uint32_t slot = s_Data->TextureSlotIndex;
        s_Data->TextureSlots[slot] = texture;
        s_Data->TextureSlotIndex++;
        return static_cast<float>(slot);
    }

    void Renderer2D::Configure2DMaterial(const std::shared_ptr<Material>& material) {
        if (!material) {
            return;
        }

        auto& state = material->GetRenderState();
        state.DepthTest = false;
        state.DepthWrite = false;
        state.Blending = true;
        state.BlendSrc = BlendFactor::SrcAlpha;
        state.BlendDst = BlendFactor::OneMinusSrcAlpha;
        state.CullFace = false;
    }

    void Renderer2D::SubmitImmediateQuadDefault(const glm::mat4& transform,
                                                const glm::vec2* texCoords,
                                                const std::shared_ptr<Texture2D>& texture,
                                                float tiling,
                                                const glm::vec4& tint) {
        FlushAll();

        QuadVertex quadVertices[4];
        const float texIndex = texture ? 1.0f : 0.0f;
        for (int i = 0; i < 4; i++) {
            quadVertices[i].Position = transform * s_Data->QuadVertexPositions[i];
            quadVertices[i].Color = tint;
            quadVertices[i].TexCoord = texCoords[i];
            quadVertices[i].TexIndex = texIndex;
            quadVertices[i].TilingFactor = tiling;
        }

        QuadVertex vertices[6] = {
            quadVertices[0], quadVertices[1], quadVertices[2],
            quadVertices[2], quadVertices[3], quadVertices[0]
        };
        s_Data->ImmediateQuadVBO->SetData(vertices, sizeof(vertices));

        if (texture) {
            Configure2DMaterial(s_Data->ImmediateQuadMaterial);
            RenderCommand::SetRenderState(s_Data->ImmediateQuadMaterial->GetRenderState());

            auto shader = s_Data->ImmediateQuadMaterial->GetShader();
            shader->Bind();
            shader->SetUniformMat4("u_ViewProjection", Renderer::GetViewProjection());
            shader->SetUniformMat4("u_Transform", glm::mat4(1.0f));
            texture->Bind(0);
            shader->SetUniform1i("u_Texture", 0);
        }
        else {
            Configure2DMaterial(s_Data->ImmediateColorMaterial);
            RenderCommand::SetRenderState(s_Data->ImmediateColorMaterial->GetRenderState());

            auto shader = s_Data->ImmediateColorMaterial->GetShader();
            shader->Bind();
            shader->SetUniformMat4("u_ViewProjection", Renderer::GetViewProjection());
            shader->SetUniformMat4("u_Transform", glm::mat4(1.0f));
        }

        RenderCommand::DrawArrays(s_Data->ImmediateQuadVAO, 6);
    }

    void Renderer2D::SubmitImmediateQuad(const glm::mat4& transform,
                                         const glm::vec2* texCoords,
                                         const std::shared_ptr<Texture2D>& texture,
                                         float tiling,
                                         const glm::vec4& tint,
                                         const std::shared_ptr<Material>& material) {
        if (!material) {
            return;
        }

        FlushAll();

        QuadVertex quadVertices[4];
        for (int i = 0; i < 4; i++) {
            quadVertices[i].Position = transform * s_Data->QuadVertexPositions[i];
            quadVertices[i].Color = tint;
            quadVertices[i].TexCoord = texCoords[i];
            quadVertices[i].TexIndex = 0.0f;
            quadVertices[i].TilingFactor = tiling;
        }

        QuadVertex vertices[6] = {
            quadVertices[0], quadVertices[1], quadVertices[2],
            quadVertices[2], quadVertices[3], quadVertices[0]
        };
        s_Data->ImmediateQuadVBO->SetData(vertices, sizeof(vertices));

        Configure2DMaterial(material);
        material->Set("u_ViewProjection", Renderer::GetViewProjection());
        material->Set("u_Transform", glm::mat4(1.0f));
        if (texture && material->GetShader() && material->GetShader()->HasUniform("u_Texture")) {
            material->SetTexture("u_Texture", texture);
        }
        material->Bind();

        RenderCommand::DrawArrays(s_Data->ImmediateQuadVAO, 6);
    }

    void Renderer2D::FlushQuads() {
        const uint32_t vertexCount = static_cast<uint32_t>(s_Data->QuadBufferPtr - s_Data->QuadBufferBase);
        if (vertexCount == 0) {
            return;
        }

        const uint32_t size = vertexCount * sizeof(QuadVertex);
        s_Data->QuadVBO->SetData(s_Data->QuadBufferBase, size);

        for (uint32_t i = 0; i < s_Data->TextureSlotIndex; i++) {
            if (s_Data->TextureSlots[i]) {
                s_Data->TextureSlots[i]->Bind(static_cast<int>(i));
            }
        }

        Renderer::Submit(
            s_Data->QuadVAO,
            s_Data->QuadMaterial,
            (vertexCount / 4) * 6,
            glm::mat4(1.0f)
        );
    }

    void Renderer2D::FlushLines() {
        const uint32_t vertexCount = static_cast<uint32_t>(s_Data->LineBufferPtr - s_Data->LineBufferBase);
        if (vertexCount == 0) {
            return;
        }

        const uint32_t size = vertexCount * sizeof(LineVertex);
        s_Data->LineVBO->SetData(s_Data->LineBufferBase, size);

        Renderer::SubmitLines(
            s_Data->LineVAO,
            s_Data->LineMaterial,
            vertexCount
        );
    }

    void Renderer2D::FlushCircles() {
        const uint32_t vertexCount = static_cast<uint32_t>(s_Data->CircleBufferPtr - s_Data->CircleBufferBase);
        if (vertexCount == 0) {
            return;
        }

        const uint32_t size = vertexCount * sizeof(CircleVertex);
        s_Data->CircleVBO->SetData(s_Data->CircleBufferBase, size);

        Renderer::Submit(
            s_Data->CircleVAO,
            s_Data->CircleMaterial,
            (vertexCount / 4) * 6,
            glm::mat4(1.0f)
        );
    }

    void Renderer2D::DrawQuad(const glm::vec2& pos, const glm::vec2& size, const glm::vec4& color, float z) {
        DrawQuad(glm::vec3(pos, z), size, color);
    }

    void Renderer2D::DrawQuad(const glm::vec3& pos, const glm::vec2& size, const glm::vec4& color) {
        glm::mat4 transform =
            glm::translate(glm::mat4(1.0f), pos) *
            glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });

        DrawQuad(transform, color);
    }

    void Renderer2D::DrawQuad(const glm::mat4& transform, const glm::vec4& color) {
        static const glm::vec2 texCoords[4] = {
            { 0.0f, 0.0f },
            { 1.0f, 0.0f },
            { 1.0f, 1.0f },
            { 0.0f, 1.0f }
        };

        SubmitImmediateQuadDefault(transform, texCoords, nullptr, 1.0f, color);
    }

    void Renderer2D::DrawQuad(const glm::vec2& pos,
                              const glm::vec2& size,
                              const std::shared_ptr<Texture2D>& texture,
                              float tiling,
                              const glm::vec4& tint,
                              float z) {
        DrawQuad(glm::vec3(pos, z), size, texture, tiling, tint);
    }

    void Renderer2D::DrawQuad(const glm::vec3& pos,
                              const glm::vec2& size,
                              const std::shared_ptr<Texture2D>& texture,
                              float tiling,
                              const glm::vec4& tint) {
        glm::mat4 transform =
            glm::translate(glm::mat4(1.0f), pos) *
            glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });

        DrawQuad(transform, texture, tiling, tint);
    }

    void Renderer2D::DrawQuad(const glm::mat4& transform,
                              const std::shared_ptr<Texture2D>& texture,
                              float tiling,
                              const glm::vec4& tint) {
        static const glm::vec2 texCoords[4] = {
            { 0.0f, 0.0f },
            { 1.0f, 0.0f },
            { 1.0f, 1.0f },
            { 0.0f, 1.0f }
        };

        SubmitImmediateQuadDefault(transform, texCoords, texture, tiling, tint);
    }

    void Renderer2D::DrawQuad(const glm::mat4& transform,
                              const std::shared_ptr<Texture2D>& texture,
                              const std::shared_ptr<Material>& materialOverride,
                              float tiling,
                              const glm::vec4& tint) {
        static const glm::vec2 texCoords[4] = {
            { 0.0f, 0.0f },
            { 1.0f, 0.0f },
            { 1.0f, 1.0f },
            { 0.0f, 1.0f }
        };

        if (!materialOverride) {
            DrawQuad(transform, texture, tiling, tint);
            return;
        }

        SubmitImmediateQuad(transform, texCoords, texture, tiling, tint, materialOverride);
    }

    void Renderer2D::DrawSprite(const glm::mat4& transform, const Sprite& sprite, const glm::vec4& tint) {
        const glm::vec2 uv[4] = {
            { sprite.UVMin.x, sprite.UVMin.y },
            { sprite.UVMax.x, sprite.UVMin.y },
            { sprite.UVMax.x, sprite.UVMax.y },
            { sprite.UVMin.x, sprite.UVMax.y }
        };

        SubmitImmediateQuadDefault(transform, uv, sprite.Texture, 1.0f, tint);
    }

    void Renderer2D::DrawSprite(const glm::mat4& transform,
                                const Sprite& sprite,
                                const glm::vec4& tint,
                                const std::shared_ptr<Material>& materialOverride) {
        const glm::vec2 uv[4] = {
            { sprite.UVMin.x, sprite.UVMin.y },
            { sprite.UVMax.x, sprite.UVMin.y },
            { sprite.UVMax.x, sprite.UVMax.y },
            { sprite.UVMin.x, sprite.UVMax.y }
        };

        if (!materialOverride) {
            DrawSprite(transform, sprite, tint);
            return;
        }

        SubmitImmediateQuad(transform, uv, sprite.Texture, 1.0f, tint, materialOverride);
    }

    void Renderer2D::DrawSkinned(const glm::mat4& transform,
                                 const SkinnedMesh2D& mesh,
                                 const SkeletonPose2D& pose,
                                 const glm::vec4& tint,
                                 const std::shared_ptr<Material>& materialOverride) {
        if (mesh.Vertices.empty() || mesh.Indices.empty()) {
            return;
        }

        if (mesh.Vertices.size() > RendererData::MaxVertices || mesh.Indices.size() > RendererData::MaxIndices) {
            return;
        }

        FlushAll();

        std::vector<SkinnedVertex2D> vertices = mesh.Vertices;
        for (auto& vertex : vertices) {
            vertex.Color *= tint;
        }

        s_Data->SkinnedVBO->SetData(vertices.data(), static_cast<uint32_t>(vertices.size() * sizeof(SkinnedVertex2D)));
        s_Data->SkinnedIBO = IndexBuffer::Create(const_cast<uint32_t*>(mesh.Indices.data()), static_cast<uint32_t>(mesh.Indices.size()));
        s_Data->SkinnedVAO->SetIndexBuffer(s_Data->SkinnedIBO);

        auto material = materialOverride ? materialOverride : s_Data->SkinnedMaterial;
        Configure2DMaterial(material);

        auto shader = material->GetShader();
        RenderCommand::SetRenderState(material->GetRenderState());
        shader->Bind();
        shader->SetUniformMat4("u_ViewProjection", Renderer::GetViewProjection());
        shader->SetUniformMat4("u_Transform", transform);

        const uint32_t boneCount = pose.BoneCount < MaxBones ? pose.BoneCount : MaxBones;
        for (uint32_t i = 0; i < boneCount; i++) {
            shader->SetUniformMat4("u_BoneTransforms[" + std::to_string(i) + "]", pose.BoneTransforms[i]);
        }

        if (mesh.Texture && shader->HasUniform("u_Texture")) {
            mesh.Texture->Bind(0);
            shader->SetUniform1i("u_Texture", 0);
        }

        if (materialOverride) {
            material->Bind();
        }
        RenderCommand::DrawIndexed(s_Data->SkinnedVAO, static_cast<uint32_t>(mesh.Indices.size()));
    }

    void Renderer2D::DrawLine(const glm::vec3& p0, const glm::vec3& p1, const glm::vec4& color) {
        DrawLine(p0, p1, color, 1.5f);
    }

    void Renderer2D::DrawLine(const glm::vec3& p0, const glm::vec3& p1, const glm::vec4& color, float thickness) {
        DrawLine(p0, p1, color, thickness, LineCap::Butt);
    }

    void Renderer2D::DrawLine(const glm::vec3& p0,
                              const glm::vec3& p1,
                              const glm::vec4& color,
                              float thickness,
                              LineCap cap) {
        if (thickness <= 0.0f) {
            return;
        }

        const glm::vec2 delta = glm::vec2(p1) - glm::vec2(p0);
        const float length = glm::length(delta);
        if (length <= kEpsilon) {
            DrawCircle(p0, thickness * 0.5f, 0.0f, color);
            return;
        }

        glm::vec3 start = p0;
        glm::vec3 end = p1;
        const glm::vec2 dir = delta / length;
        const float halfThickness = thickness * 0.5f;

        if (cap == LineCap::Square) {
            start -= glm::vec3(dir * halfThickness, 0.0f);
            end += glm::vec3(dir * halfThickness, 0.0f);
        }

        const glm::vec2 finalDelta = glm::vec2(end) - glm::vec2(start);
        const float finalLength = glm::length(finalDelta);
        const glm::vec3 center = (start + end) * 0.5f;
        const float angle = std::atan2(finalDelta.y, finalDelta.x);

        glm::mat4 transform =
            glm::translate(glm::mat4(1.0f), center) *
            glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0.0f, 0.0f, 1.0f)) *
            glm::scale(glm::mat4(1.0f), glm::vec3(finalLength, thickness, 1.0f));

        DrawQuad(transform, color);

        if (cap == LineCap::Round) {
            DrawCircle(p0, halfThickness, 0.0f, color);
            DrawCircle(p1, halfThickness, 0.0f, color);
        }
    }

    void Renderer2D::DrawLineStrip(const std::vector<glm::vec3>& points,
                                   const glm::vec4& color,
                                   float thickness,
                                   bool closed,
                                   LineCap cap,
                                   LineJoin join,
                                   float miterLimit) {
        if (points.size() < 2 || thickness <= 0.0f) {
            return;
        }

        const size_t pointCount = points.size();
        const size_t segmentCount = closed ? pointCount : pointCount - 1;
        const LineCap segmentCap = closed ? LineCap::Butt : cap;

        for (size_t i = 0; i < segmentCount; i++) {
            const glm::vec3& pointA = points[i];
            const glm::vec3& pointB = points[(i + 1) % pointCount];
            DrawLine(pointA, pointB, color, thickness, segmentCap);
        }
    }

    void Renderer2D::DrawRect(const glm::vec3& pos, const glm::vec2& size, const glm::vec4& color, float z, float thickness) {
        glm::vec3 p0 = pos + glm::vec3(0.0f, 0.0f, z);
        glm::vec3 p1 = pos + glm::vec3(size.x, 0.0f, z);
        glm::vec3 p2 = pos + glm::vec3(size.x, size.y, z);
        glm::vec3 p3 = pos + glm::vec3(0.0f, size.y, z);

        DrawLine(p0, p1, color, thickness, LineCap::Butt);
        DrawLine(p1, p2, color, thickness, LineCap::Butt);
        DrawLine(p2, p3, color, thickness, LineCap::Butt);
        DrawLine(p3, p0, color, thickness, LineCap::Butt);
    }

    void Renderer2D::DrawRect(const glm::mat4& transform, const glm::vec4& color, float thickness) {
        glm::vec3 points[4];
        for (int i = 0; i < 4; i++) {
            points[i] = transform * s_Data->QuadVertexPositions[i];
        }

        DrawLine(points[0], points[1], color, thickness, LineCap::Butt);
        DrawLine(points[1], points[2], color, thickness, LineCap::Butt);
        DrawLine(points[2], points[3], color, thickness, LineCap::Butt);
        DrawLine(points[3], points[0], color, thickness, LineCap::Butt);
    }

    void Renderer2D::DrawCircle(const glm::vec2& pos, float radius, float thickness, const glm::vec4& color, float z) {
        DrawCircle(glm::vec3(pos, z), radius, thickness, color);
    }

    void Renderer2D::DrawCircle(const glm::vec3& pos, float radius, float thickness, const glm::vec4& color) {
        glm::mat4 transform =
            glm::translate(glm::mat4(1.0f), pos) *
            glm::scale(glm::mat4(1.0f), glm::vec3(radius * 2.0f, radius * 2.0f, 1.0f));

        DrawCircle(transform, thickness, color);
    }

    void Renderer2D::DrawCircle(const glm::mat4& transform, float thickness, const glm::vec4& color) {
        EnsureCircleCapacity();

        for (int i = 0; i < 4; i++) {
            s_Data->CircleBufferPtr->Position = transform * s_Data->QuadVertexPositions[i];
            s_Data->CircleBufferPtr->LocalPosition = glm::vec3(glm::vec2(s_Data->QuadVertexPositions[i]) * 2.0f, 0.0f);
            s_Data->CircleBufferPtr->Color = color;
            s_Data->CircleBufferPtr->Thickness = thickness;
            s_Data->CircleBufferPtr++;
        }
    }

} // namespace axiom
