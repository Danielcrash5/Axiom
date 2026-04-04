#include "axiom/renderer/Renderer2D.h"
#include "axiom/renderer/Renderer.h"

#include <glm/gtc/matrix_transform.hpp>

namespace axiom {

    Renderer2D::RendererData* Renderer2D::s_Data = nullptr;

    void Renderer2D::Init() {
        s_Data = new RendererData();

        // -------------------------
        // Quad Setup
        // -------------------------
        s_Data->QuadVAO = VertexArray::Create();
        s_Data->QuadVBO = VertexBuffer::Create(
            RendererData::MaxVertices * sizeof(QuadVertex),
            BufferUsage::Dynamic
        );

        s_Data->QuadVBO->SetLayout({
            { ShaderDataType::Vec3, "a_Position" },
            { ShaderDataType::Vec4, "a_Color" },
            { ShaderDataType::Vec2, "a_TexCoord" },
            { ShaderDataType::Float,  "a_TexIndex" },
            { ShaderDataType::Float,  "a_TilingFactor" }
                                   });

        s_Data->QuadVAO->AddVertexBuffer(s_Data->QuadVBO);

        uint32_t* indices = new uint32_t[RendererData::MaxIndices];
        uint32_t offset = 0;

        for (uint32_t i = 0; i < RendererData::MaxIndices; i += 6) {
            indices[i + 0] = offset + 0;
            indices[i + 1] = offset + 1;
            indices[i + 2] = offset + 2;

            indices[i + 3] = offset + 2;
            indices[i + 4] = offset + 3;
            indices[i + 5] = offset + 0;

            offset += 4;
        }

        s_Data->QuadIBO = IndexBuffer::Create(indices, RendererData::MaxIndices);
        s_Data->QuadVAO->SetIndexBuffer(s_Data->QuadIBO);
        delete[] indices;

        s_Data->QuadBufferBase = new QuadVertex[RendererData::MaxVertices];

        // -------------------------
        // Line Setup
        // -------------------------
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
        s_Data->LineBufferBase = new LineVertex[RendererData::MaxVertices];

        // -------------------------
        // Circle Setup
        // -------------------------
        s_Data->CircleVAO = VertexArray::Create();
        s_Data->CircleVBO = VertexBuffer::Create(
            RendererData::MaxVertices * sizeof(CircleVertex),
            BufferUsage::Dynamic
        );

        s_Data->CircleVBO->SetLayout({
            { ShaderDataType::Vec3, "a_Position" },
            { ShaderDataType::Vec4, "a_Color" },
            { ShaderDataType::Float,  "a_Thickness" }
                                     });

        s_Data->CircleVAO->AddVertexBuffer(s_Data->CircleVBO);
        s_Data->CircleBufferBase = new CircleVertex[RendererData::MaxVertices];

        // Default quad positions
        s_Data->QuadVertexPositions[0] = { -0.5f, -0.5f, 0.0f, 1.0f };
        s_Data->QuadVertexPositions[1] = { 0.5f, -0.5f, 0.0f, 1.0f };
        s_Data->QuadVertexPositions[2] = { 0.5f,  0.5f, 0.0f, 1.0f };
        s_Data->QuadVertexPositions[3] = { -0.5f,  0.5f, 0.0f, 1.0f };

        StartBatch();
    }

    void Renderer2D::Shutdown() {
        delete[] s_Data->QuadBufferBase;
        delete[] s_Data->LineBufferBase;
        delete[] s_Data->CircleBufferBase;
        delete s_Data;
    }

    void Renderer2D::BeginScene() {
        StartBatch();
    }

    void Renderer2D::EndScene() {
        FlushQuads();
        FlushLines();
        FlushCircles();
    }

    void Renderer2D::StartBatch() {
        s_Data->QuadBufferPtr = s_Data->QuadBufferBase;
        s_Data->LineBufferPtr = s_Data->LineBufferBase;
        s_Data->CircleBufferPtr = s_Data->CircleBufferBase;

        s_Data->TextureSlotIndex = 1;
    }

    void Renderer2D::FlushQuads() {
        uint32_t size = (uint8_t*)s_Data->QuadBufferPtr - (uint8_t*)s_Data->QuadBufferBase;
        if (size == 0) return;

        s_Data->QuadVBO->SetData(s_Data->QuadBufferBase, size);

        for (uint32_t i = 0; i < s_Data->TextureSlotIndex; i++)
            s_Data->TextureSlots[i]->Bind(i);

        Renderer::Submit(
            s_Data->QuadVAO,
            s_Data->QuadMaterial,
            size / sizeof(QuadVertex) * 6,
            glm::mat4(1.0f)
        );
    }

    void Renderer2D::FlushLines() {
        uint32_t size = (uint8_t*)s_Data->LineBufferPtr - (uint8_t*)s_Data->LineBufferBase;
        if (size == 0) return;

        s_Data->LineVBO->SetData(s_Data->LineBufferBase, size);

        Renderer::SubmitLines(
            s_Data->LineVAO,
            s_Data->LineMaterial,
            size / sizeof(LineVertex)
        );
    }

    void Renderer2D::FlushCircles() {
        uint32_t size = (uint8_t*)s_Data->CircleBufferPtr - (uint8_t*)s_Data->CircleBufferBase;
        if (size == 0) return;

        s_Data->CircleVBO->SetData(s_Data->CircleBufferBase, size);

        Renderer::Submit(
            s_Data->CircleVAO,
            s_Data->CircleMaterial,
            size / sizeof(CircleVertex),
            glm::mat4(1.0f)
        );
    }

    //
// ─────────────────────────────────────────────────────────────
//   QUADS (COLOR)
// ─────────────────────────────────────────────────────────────
//

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
        for (int i = 0; i < 4; i++) {
            s_Data->QuadBufferPtr->Position = transform * s_Data->QuadVertexPositions[i];
            s_Data->QuadBufferPtr->Color = color;
            s_Data->QuadBufferPtr->TexCoord = { (i == 1 || i == 2), (i >= 2) };
            s_Data->QuadBufferPtr->TexIndex = 0.0f;
            s_Data->QuadBufferPtr->TilingFactor = 1.0f;
            s_Data->QuadBufferPtr++;
        }
    }

    //
    // ─────────────────────────────────────────────────────────────
    //   QUADS (TEXTURED)
    // ─────────────────────────────────────────────────────────────
    //

    void Renderer2D::DrawQuad(
        const glm::vec2& pos,
        const glm::vec2& size,
        const std::shared_ptr<Texture2D>& texture,
        float tiling,
        const glm::vec4& tint,
        float z) {
        DrawQuad(glm::vec3(pos, z), size, texture, tiling, tint);
    }

    void Renderer2D::DrawQuad(
        const glm::mat4& transform,
        const std::shared_ptr<Texture2D>& texture,
        float tiling,
        const glm::vec4& tint) {
        float texIndex = 0.0f;

        for (uint32_t i = 1; i < s_Data->TextureSlotIndex; i++) {
            if (s_Data->TextureSlots[i].get() == texture.get()) {
                texIndex = (float)i;
                break;
            }
        }

        if (texIndex == 0.0f) {
            texIndex = (float)s_Data->TextureSlotIndex;
            s_Data->TextureSlots[s_Data->TextureSlotIndex] = texture;
            s_Data->TextureSlotIndex++;
        }

        for (int i = 0; i < 4; i++) {
            s_Data->QuadBufferPtr->Position = transform * s_Data->QuadVertexPositions[i];
            s_Data->QuadBufferPtr->Color = tint;
            s_Data->QuadBufferPtr->TexCoord = { (i == 1 || i == 2), (i >= 2) };
            s_Data->QuadBufferPtr->TexIndex = texIndex;
            s_Data->QuadBufferPtr->TilingFactor = tiling;
            s_Data->QuadBufferPtr++;
        }
    }

    //
    // ─────────────────────────────────────────────────────────────
    //   SPRITES (UV + Pivot)
    // ─────────────────────────────────────────────────────────────
    //

    void Renderer2D::DrawSprite(const glm::mat4& transform, const Sprite& sprite, const glm::vec4& tint) {
        float texIndex = 0.0f;

        auto texture = sprite.Texture;

        for (uint32_t i = 1; i < s_Data->TextureSlotIndex; i++) {
            if (s_Data->TextureSlots[i].get() == texture.get()) {
                texIndex = (float)i;
                break;
            }
        }

        if (texIndex == 0.0f) {
            texIndex = (float)s_Data->TextureSlotIndex;
            s_Data->TextureSlots[s_Data->TextureSlotIndex] = texture;
            s_Data->TextureSlotIndex++;
        }

        glm::vec2 uv[4] = {
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

    //
    // ─────────────────────────────────────────────────────────────
    //   LINES
    // ─────────────────────────────────────────────────────────────
    //

    void Renderer2D::DrawLine(const glm::vec3& p0, const glm::vec3& p1, const glm::vec4& color) {
        s_Data->LineBufferPtr->Position = p0;
        s_Data->LineBufferPtr->Color = color;
        s_Data->LineBufferPtr++;

        s_Data->LineBufferPtr->Position = p1;
        s_Data->LineBufferPtr->Color = color;
        s_Data->LineBufferPtr++;
    }

    //
    // ─────────────────────────────────────────────────────────────
    //   RECTANGLES
    // ─────────────────────────────────────────────────────────────
    //

    void Renderer2D::DrawRect(const glm::vec3& pos, const glm::vec2& size, const glm::vec4& color, float z) {
        glm::vec3 p0 = pos + glm::vec3(0, 0, z);
        glm::vec3 p1 = pos + glm::vec3(size.x, 0, z);
        glm::vec3 p2 = pos + glm::vec3(size.x, size.y, z);
        glm::vec3 p3 = pos + glm::vec3(0, size.y, z);

        DrawLine(p0, p1, color);
        DrawLine(p1, p2, color);
        DrawLine(p2, p3, color);
        DrawLine(p3, p0, color);
    }

    void Renderer2D::DrawRect(const glm::mat4& transform, const glm::vec4& color) {
        glm::vec3 p[4];
        for (int i = 0; i < 4; i++)
            p[i] = transform * s_Data->QuadVertexPositions[i];

        DrawLine(p[0], p[1], color);
        DrawLine(p[1], p[2], color);
        DrawLine(p[2], p[3], color);
        DrawLine(p[3], p[0], color);
    }

    //
    // ─────────────────────────────────────────────────────────────
    //   CIRCLES
    // ─────────────────────────────────────────────────────────────
    //

    void Renderer2D::DrawCircle(const glm::mat4& transform, float thickness, const glm::vec4& color) {
        for (int i = 0; i < 4; i++) {
            s_Data->CircleBufferPtr->Position = transform * s_Data->QuadVertexPositions[i];
            s_Data->CircleBufferPtr->Color = color;
            s_Data->CircleBufferPtr->Thickness = thickness;
            s_Data->CircleBufferPtr++;
        }
    }


} // namespace axiom