#include "axiom/renderer/Renderer2D.h"
#include "axiom/renderer/Buffer.h"
#include "axiom/renderer/VertexArray.h"
#include "axiom/renderer/Shader.h"
#include "axiom/renderer/Material.h"
#include "axiom/renderer/Renderer.h"

#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>

namespace axiom {

    // ===================== CONSTANTS =====================

    static const uint32_t MaxQuads = 1000;
    static const uint32_t MaxVertices = MaxQuads * 4;
    static const uint32_t MaxIndices = MaxQuads * 6;

    // ===================== VERTEX =====================

    struct QuadVertex {
        glm::vec3 Position;
        glm::vec4 Color;
        glm::vec2 TexCoord;
        float TexIndex;
    };

    // ===================== DATA =====================

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

        // Indices
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

        s_Data.VertexBufferBase = new QuadVertex[MaxVertices];
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

    // ===================== END =====================

    void Renderer2D::End() {
        Flush();
    }

    // ===================== START BATCH =====================

    void Renderer2D::StartBatch() {
        s_Data.QuadIndexCount = 0;
        s_Data.VertexBufferPtr = s_Data.VertexBufferBase;
    }

    // ===================== DRAW =====================

    void Renderer2D::DrawQuad(
        const glm::vec3& position,
        const glm::vec2& size,
        const glm::vec4& color) {
        if (s_Data.QuadIndexCount >= MaxIndices)
            Flush();

        glm::vec3 positions[4] = {
            { -0.5f, -0.5f, 0.0f },
            {  0.5f, -0.5f, 0.0f },
            {  0.5f,  0.5f, 0.0f },
            { -0.5f,  0.5f, 0.0f }
        };

        glm::vec2 texCoords[4] = {
            {0,0},{1,0},{1,1},{0,1}
        };

        glm::mat4 transform =
            glm::translate(glm::mat4(1.0f), position) *
            glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });

        for (int i = 0; i < 4; i++) {
            s_Data.VertexBufferPtr->Position = transform * glm::vec4(positions[i], 1.0f);
            s_Data.VertexBufferPtr->Color = color;
            s_Data.VertexBufferPtr->TexCoord = texCoords[i];
            s_Data.VertexBufferPtr->TexIndex = 0.0f;
            s_Data.VertexBufferPtr++;
        }

        s_Data.QuadIndexCount += 6;
    }

    // ===================== FLUSH =====================

    void Renderer2D::Flush() {
        if (s_Data.QuadIndexCount == 0)
            return;

        uint32_t size = (uint8_t*)s_Data.VertexBufferPtr - (uint8_t*)s_Data.VertexBufferBase;

        s_Data.VBO->Bind();
        glBufferSubData(GL_ARRAY_BUFFER, 0, size, s_Data.VertexBufferBase);

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

}