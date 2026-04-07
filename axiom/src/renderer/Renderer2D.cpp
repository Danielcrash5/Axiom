#include "axiom/renderer/Renderer2D.h"
#include "axiom/renderer/Renderer.h"
#include "axiom/renderer/RenderCommand.h"
#include "axiom/assets/AssetManager.h"

#include <glm/gtc/matrix_transform.hpp>
#include <string>

namespace axiom {

    Renderer2D::RendererData* Renderer2D::s_Data = nullptr;

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
        s_Data->QuadVBO->SetLayout({
            { ShaderDataType::Vec3, "a_Position" },
            { ShaderDataType::Vec4, "a_Color" },
            { ShaderDataType::Vec2, "a_TexCoord" },
            { ShaderDataType::Float, "a_TexIndex" },
            { ShaderDataType::Float, "a_TilingFactor" }
        });
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

        s_Data->LineVAO = VertexArray::Create();
        s_Data->LineVBO = VertexBuffer::Create(
            RendererData::MaxVertices * sizeof(LineVertex),
            BufferUsage::Dynamic
        );
        s_Data->LineVBO->SetLayout({
            { ShaderDataType::Vec3, "a_Position" },
            { ShaderDataType::Vec4, "a_Color" }
        });
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
        s_Data->CircleVBO->SetLayout({
            { ShaderDataType::Vec3, "a_Position" },
            { ShaderDataType::Vec4, "a_Color" },
            { ShaderDataType::Float, "a_Thickness" }
        });
        s_Data->CircleVAO->AddVertexBuffer(s_Data->CircleVBO);
        s_Data->CircleVAO->SetIndexBuffer(s_Data->QuadIBO);
        s_Data->CircleBufferBase = new CircleVertex[RendererData::MaxVertices];

        s_Data->SkinnedVAO = VertexArray::Create();
        s_Data->SkinnedVBO = VertexBuffer::Create(
            RendererData::MaxVertices * sizeof(SkinnedVertex2D),
            BufferUsage::Dynamic
        );
        s_Data->SkinnedVBO->SetLayout({
            { ShaderDataType::Vec3, "a_Position" },
            { ShaderDataType::Vec4, "a_Color" },
            { ShaderDataType::Vec2, "a_TexCoord" },
            { ShaderDataType::Vec4, "a_BoneIndices" },
            { ShaderDataType::Vec4, "a_BoneWeights" }
        });
        s_Data->SkinnedVAO->AddVertexBuffer(s_Data->SkinnedVBO);

        auto quadShader = AssetManager::Get<Shader>("engine://shaders/Renderer2D/Quad.glsl");
        auto lineShader = AssetManager::Get<Shader>("engine://shaders/Renderer2D/Line.glsl");
        auto circleShader = AssetManager::Get<Shader>("engine://shaders/Renderer2D/Circle.glsl");
        auto skinnedShader = AssetManager::Get<Shader>("engine://shaders/Renderer2D/Skinned.glsl");

        s_Data->QuadMaterial = std::make_shared<Material>(quadShader);
        s_Data->LineMaterial = std::make_shared<Material>(lineShader);
        s_Data->CircleMaterial = std::make_shared<Material>(circleShader);
        s_Data->SkinnedMaterial = std::make_shared<Material>(skinnedShader);

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

        QuadVertex vertices[4];
        for (int i = 0; i < 4; i++) {
            vertices[i].Position = transform * s_Data->QuadVertexPositions[i];
            vertices[i].Color = tint;
            vertices[i].TexCoord = texCoords[i];
            vertices[i].TexIndex = 0.0f;
            vertices[i].TilingFactor = tiling;
        }

        s_Data->QuadVBO->SetData(vertices, sizeof(vertices));

        material->Set("u_ViewProjection", Renderer::GetViewProjection());
        material->Set("u_Transform", glm::mat4(1.0f));
        if (texture) {
            material->SetTexture("u_Texture", texture);
        }
        material->Bind();

        RenderCommand::DrawIndexed(s_Data->QuadVAO, 6);
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
        EnsureQuadCapacity();

        for (int i = 0; i < 4; i++) {
            s_Data->QuadBufferPtr->Position = transform * s_Data->QuadVertexPositions[i];
            s_Data->QuadBufferPtr->Color = color;
            s_Data->QuadBufferPtr->TexCoord = { (i == 1 || i == 2) ? 1.0f : 0.0f, (i >= 2) ? 1.0f : 0.0f };
            s_Data->QuadBufferPtr->TexIndex = 0.0f;
            s_Data->QuadBufferPtr->TilingFactor = 1.0f;
            s_Data->QuadBufferPtr++;
        }
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
        EnsureQuadCapacity();
        const float texIndex = AcquireTextureSlot(texture);

        for (int i = 0; i < 4; i++) {
            s_Data->QuadBufferPtr->Position = transform * s_Data->QuadVertexPositions[i];
            s_Data->QuadBufferPtr->Color = tint;
            s_Data->QuadBufferPtr->TexCoord = { (i == 1 || i == 2) ? 1.0f : 0.0f, (i >= 2) ? 1.0f : 0.0f };
            s_Data->QuadBufferPtr->TexIndex = texIndex;
            s_Data->QuadBufferPtr->TilingFactor = tiling;
            s_Data->QuadBufferPtr++;
        }
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
        EnsureQuadCapacity();
        const float texIndex = AcquireTextureSlot(sprite.Texture);

        const glm::vec2 uv[4] = {
            { sprite.UVMin.x, sprite.UVMin.y },
            { sprite.UVMax.x, sprite.UVMin.y },
            { sprite.UVMax.x, sprite.UVMax.y },
            { sprite.UVMin.x, sprite.UVMax.y }
        };

        for (int i = 0; i < 4; i++) {
            s_Data->QuadBufferPtr->Position = transform * s_Data->QuadVertexPositions[i];
            s_Data->QuadBufferPtr->Color = tint;
            s_Data->QuadBufferPtr->TexCoord = uv[i];
            s_Data->QuadBufferPtr->TexIndex = texIndex;
            s_Data->QuadBufferPtr->TilingFactor = 1.0f;
            s_Data->QuadBufferPtr++;
        }
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
        material->Set("u_ViewProjection", Renderer::GetViewProjection());
        material->Set("u_Transform", transform);
        if (mesh.Texture) {
            material->SetTexture("u_Texture", mesh.Texture);
        }

        auto shader = material->GetShader();
        shader->Bind();

        const uint32_t boneCount = pose.BoneCount < MaxBones ? pose.BoneCount : MaxBones;
        for (uint32_t i = 0; i < boneCount; i++) {
            shader->SetUniformMat4("u_BoneTransforms[" + std::to_string(i) + "]", pose.BoneTransforms[i]);
        }

        material->Bind();
        RenderCommand::DrawIndexed(s_Data->SkinnedVAO, static_cast<uint32_t>(mesh.Indices.size()));
    }

    void Renderer2D::DrawLine(const glm::vec3& p0, const glm::vec3& p1, const glm::vec4& color) {
        EnsureLineCapacity();

        s_Data->LineBufferPtr->Position = p0;
        s_Data->LineBufferPtr->Color = color;
        s_Data->LineBufferPtr++;

        s_Data->LineBufferPtr->Position = p1;
        s_Data->LineBufferPtr->Color = color;
        s_Data->LineBufferPtr++;
    }

    void Renderer2D::DrawRect(const glm::vec3& pos, const glm::vec2& size, const glm::vec4& color, float z) {
        glm::vec3 p0 = pos + glm::vec3(0.0f, 0.0f, z);
        glm::vec3 p1 = pos + glm::vec3(size.x, 0.0f, z);
        glm::vec3 p2 = pos + glm::vec3(size.x, size.y, z);
        glm::vec3 p3 = pos + glm::vec3(0.0f, size.y, z);

        DrawLine(p0, p1, color);
        DrawLine(p1, p2, color);
        DrawLine(p2, p3, color);
        DrawLine(p3, p0, color);
    }

    void Renderer2D::DrawRect(const glm::mat4& transform, const glm::vec4& color) {
        glm::vec3 p[4];
        for (int i = 0; i < 4; i++) {
            p[i] = transform * s_Data->QuadVertexPositions[i];
        }

        DrawLine(p[0], p[1], color);
        DrawLine(p[1], p[2], color);
        DrawLine(p[2], p[3], color);
        DrawLine(p[3], p[0], color);
    }

    void Renderer2D::DrawCircle(const glm::mat4& transform, float thickness, const glm::vec4& color) {
        EnsureCircleCapacity();

        for (int i = 0; i < 4; i++) {
            s_Data->CircleBufferPtr->Position = transform * s_Data->QuadVertexPositions[i];
            s_Data->CircleBufferPtr->Color = color;
            s_Data->CircleBufferPtr->Thickness = thickness;
            s_Data->CircleBufferPtr++;
        }
    }

} // namespace axiom
