#include "axiom/renderer/Renderer2D.h"
#include "axiom/renderer/Buffer.h"
#include "axiom/renderer/VertexArray.h"
#include "axiom/renderer/Shader.h"
#include "axiom/renderer/Material.h"
#include "axiom/renderer/Renderer.h"
#include "axiom/core/Logger.h"

#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>

namespace axiom {

    static const uint32_t MaxQuads = 1000;
    static const uint32_t MaxVertices = MaxQuads * 4;
    static const uint32_t MaxIndices = MaxQuads * 6;
    static const uint32_t MaxTextureSlots = 32;

    struct QuadVertex {
        glm::vec3 Position;
        glm::vec4 Color;
        glm::vec2 TexCoord;
        float TexIndex;
    };

    struct Renderer2DData {
        std::shared_ptr<VertexArray> VAO;
        std::shared_ptr<VertexBuffer> VBO;
        std::shared_ptr<IndexBuffer> IBO;
        std::shared_ptr<Shader> Shader;
        std::shared_ptr<Material> Material;

        uint32_t QuadIndexCount = 0;

        QuadVertex* VertexBufferBase = nullptr;
        QuadVertex* VertexBufferPtr = nullptr;

        glm::mat4 ViewProj;

        std::shared_ptr<Texture> TextureSlots[MaxTextureSlots];
        uint32_t TextureSlotIndex = 1; // 0 = WhiteTexture
    } s_Data;

    // ===================== INIT =====================
    void Renderer2D::Init() {
        s_Data.VAO = std::make_shared<VertexArray>();
        s_Data.VBO = std::make_shared<VertexBuffer>(nullptr, MaxVertices * sizeof(QuadVertex));

        VertexLayout layout = {
            { ShaderDataType::Vec3, "a_Position" },
            { ShaderDataType::Vec4, "a_Color" },
            { ShaderDataType::Vec2, "a_TexCoord" },
            { ShaderDataType::Float, "a_TexIndex" }
        };
        s_Data.VBO->SetLayout(layout);
        s_Data.VAO->AddVertexBuffer(s_Data.VBO);

        // Indexbuffer
        uint32_t* indices = new uint32_t[MaxIndices];
        uint32_t offset = 0;
        for (uint32_t i = 0; i < MaxIndices; i += 6) {
            indices[i + 0] = offset + 0;
            indices[i + 1] = offset + 1;
            indices[i + 2] = offset + 2;
            indices[i + 3] = offset + 2;
            indices[i + 4] = offset + 3;
            indices[i + 5] = offset + 0;
            offset += 4;
        }
        s_Data.IBO = std::make_shared<IndexBuffer>(indices, MaxIndices);
        s_Data.VAO->SetIndexBuffer(s_Data.IBO);
        delete[] indices;

        s_Data.TextureSlots[0] = std::make_shared<Texture>(1, 1);
        s_Data.TextureSlots[0]->UploadData((void*)0xFFFFFFFF, 1, 1);

        // Vertexbuffer
        s_Data.VertexBufferBase = new QuadVertex[MaxVertices];

        // Shader & Material
        ShaderDesc desc;
        desc.Vertex = "assets/shaders/renderer2d.vert.glsl";
        desc.Fragment = "assets/shaders/renderer2d.frag.glsl";
        s_Data.Shader = Shader::Create(desc);
        s_Data.Material = std::make_shared<Material>(s_Data.Shader);
    }

    // ===================== BEGIN =====================
    void Renderer2D::Begin(const glm::mat4& viewProj) {
        s_Data.ViewProj = viewProj;
        s_Data.Material->Set("u_ViewProj", s_Data.ViewProj);
        StartBatch();
    }

    void Renderer2D::End() {
        Flush();
    }

    void Renderer2D::StartBatch() {
        s_Data.QuadIndexCount = 0;
        s_Data.VertexBufferPtr = s_Data.VertexBufferBase;

        // Reset Texture Slots, außer WhiteTexture
        for (uint32_t i = 1; i < MaxTextureSlots; i++)
            s_Data.TextureSlots[i].reset();
        s_Data.TextureSlotIndex = 1;
    }

    // ===================== DRAW =====================
    // Farb-Quad
    void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color) {
        s_Data.VertexBufferPtr = s_Data.VertexBufferBase; // optional bei statischem Renderer
        DrawQuadInternal(position, size, color, 0.0f, nullptr);
    }

    // Farb + Rotation
    void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color, float rotation) {
        DrawQuadInternal(position, size, color, rotation, nullptr);
    }

    // Textur-Quad
    void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const std::shared_ptr<Texture>& texture) {
        DrawQuadInternal(position, size, glm::vec4(1.0f), 0.0f, texture);
    }

    // Textur + Rotation
    void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, float rotation, const std::shared_ptr<Texture>& texture) {
        DrawQuadInternal(position, size, glm::vec4(1.0f), rotation, texture);
    }

    // Textur + Farbe
    void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color, const std::shared_ptr<Texture>& texture) {
        DrawQuadInternal(position, size, color, 0.0f, texture);
    }

    // Textur + Farbe + Rotation
    void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color, float rotation, const std::shared_ptr<Texture>& texture) {
        DrawQuadInternal(position, size, color, rotation, texture);
    }


    void Renderer2D::DrawQuadInternal(const glm::vec3& position, const glm::vec2& size,
                                      const glm::vec4& color, float rotation,
                                      const std::shared_ptr<Texture>& texture) {
        if (s_Data.QuadIndexCount >= MaxIndices)
            Flush();

        glm::vec3 quadVertexPositions[4] = {
            { -0.5f, -0.5f, 0.0f },
            {  0.5f, -0.5f, 0.0f },
            {  0.5f,  0.5f, 0.0f },
            { -0.5f,  0.5f, 0.0f }
        };
        glm::vec2 texCoords[4] = {
            {0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f}
        };

        glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) *
            glm::rotate(glm::mat4(1.0f), rotation, glm::vec3(0, 0, 1)) *
            glm::scale(glm::mat4(1.0f), glm::vec3(size, 1.0f));

        float texIndex = 0.0f; // Default = WhiteTexture
        if (texture) {
            bool found = false;
            for (uint32_t i = 1; i < s_Data.TextureSlotIndex; i++) {
                if (s_Data.TextureSlots[i] == texture) {
                    texIndex = (float)i;
                    found = true;
                    break;
                }
            }

            if (!found) {
                if (s_Data.TextureSlotIndex >= MaxTextureSlots) {
                    Flush();
                    StartBatch();
                }
                texIndex = (float)s_Data.TextureSlotIndex;
                s_Data.TextureSlots[s_Data.TextureSlotIndex] = texture;
                s_Data.TextureSlotIndex++;
            }
        }

        for (int i = 0; i < 4; i++) {
            s_Data.VertexBufferPtr->Position = transform * glm::vec4(quadVertexPositions[i], 1.0f);
            s_Data.VertexBufferPtr->Color = color;
            s_Data.VertexBufferPtr->TexCoord = texCoords[i];
            s_Data.VertexBufferPtr->TexIndex = texIndex;
            s_Data.VertexBufferPtr++;
        }
        s_Data.QuadIndexCount += 6;
    }

    // ===================== FLUSH =====================
    void Renderer2D::Flush() {
        if (s_Data.QuadIndexCount == 0)
            return;

        s_Data.VAO->Bind();
        s_Data.VBO->Bind();

        uint32_t size = (uint8_t*)s_Data.VertexBufferPtr - (uint8_t*)s_Data.VertexBufferBase;
        glBufferSubData(GL_ARRAY_BUFFER, 0, size, s_Data.VertexBufferBase);

        // Texturen setzen
        for (uint32_t i = 0; i < s_Data.TextureSlotIndex; i++)
            s_Data.Material->Set("u_Textures[" + std::to_string(i) + "]", s_Data.TextureSlots[i]);

        s_Data.Material->Bind();

        Renderer::DrawIndexed(s_Data.VAO, s_Data.QuadIndexCount);
    }

    void Renderer2D::Shutdown() {
        delete[] s_Data.VertexBufferBase;
        s_Data.VertexBufferBase = nullptr;

        s_Data.VAO.reset();
        s_Data.VBO.reset();
        s_Data.IBO.reset();
        s_Data.Material.reset();
        s_Data.Shader.reset();
    }

} // namespace axiom