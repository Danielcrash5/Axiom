#pragma once
#include <glm/glm.hpp>
#include <array>
#include <memory>

#include "VertexArray.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "Shader.h"
#include "Texture2D.h"
#include "Material.h"
#include "Sprite.h"

namespace axiom {

    class Renderer2D {
    public:
        static void Init();
        static void Shutdown();

        static void BeginScene();
        static void EndScene();

        // --- Quads ---
        static void DrawQuad(const glm::vec2& pos, const glm::vec2& size, const glm::vec4& color, float z = 0.0f);
        static void DrawQuad(const glm::vec3& pos, const glm::vec2& size, const glm::vec4& color);
        static void DrawQuad(const glm::mat4& transform, const glm::vec4& color);

        static void DrawQuad(const glm::vec2& pos, const glm::vec2& size,
                             const std::shared_ptr<Texture2D>& texture,
                             float tiling = 1.0f,
                             const glm::vec4& tint = glm::vec4(1.0f),
                             float z = 0.0f);

        static void DrawQuad(const glm::mat4& transform,
                             const std::shared_ptr<Texture2D>& texture,
                             float tiling = 1.0f,
                             const glm::vec4& tint = glm::vec4(1.0f));

        static void DrawSprite(const glm::mat4& transform, const Sprite& sprite, const glm::vec4& tint);

        // --- Lines ---
        static void DrawLine(const glm::vec3& p0, const glm::vec3& p1, const glm::vec4& color);
        static void DrawRect(const glm::vec3& pos, const glm::vec2& size, const glm::vec4& color, float z = 0.0f);
        static void DrawRect(const glm::mat4& transform, const glm::vec4& color);

        // --- Circles ---
        static void DrawCircle(const glm::mat4& transform, float thickness, const glm::vec4& color);

    private:
        static void StartBatch();
        static void FlushQuads();
        static void FlushLines();
        static void FlushCircles();

        struct QuadVertex {
            glm::vec3 Position;
            glm::vec4 Color;
            glm::vec2 TexCoord;
            float TexIndex;
            float TilingFactor;
        };

        struct LineVertex {
            glm::vec3 Position;
            glm::vec4 Color;
        };

        struct CircleVertex {
            glm::vec3 Position;
            glm::vec4 Color;
            float Thickness;
        };

        struct RendererData {
            static constexpr uint32_t MaxQuads = 10000;
            static constexpr uint32_t MaxVertices = MaxQuads * 4;
            static constexpr uint32_t MaxIndices = MaxQuads * 6;
            static constexpr uint32_t MaxTextureSlots = 32;

            // Materials
            std::shared_ptr<Material> QuadMaterial;
            std::shared_ptr<Material> LineMaterial;
            std::shared_ptr<Material> CircleMaterial;

            // Quad buffers
            std::shared_ptr<VertexArray> QuadVAO;
            std::shared_ptr<VertexBuffer> QuadVBO;
            std::shared_ptr<IndexBuffer> QuadIBO;
            QuadVertex* QuadBufferBase = nullptr;
            QuadVertex* QuadBufferPtr = nullptr;

            // Texture slots
            std::array<std::shared_ptr<Texture2D>, MaxTextureSlots> TextureSlots;
            uint32_t TextureSlotIndex = 1;

            // Lines
            std::shared_ptr<VertexArray> LineVAO;
            std::shared_ptr<VertexBuffer> LineVBO;
            LineVertex* LineBufferBase = nullptr;
            LineVertex* LineBufferPtr = nullptr;

            // Circles
            std::shared_ptr<VertexArray> CircleVAO;
            std::shared_ptr<VertexBuffer> CircleVBO;
            CircleVertex* CircleBufferBase = nullptr;
            CircleVertex* CircleBufferPtr = nullptr;

            glm::vec4 QuadVertexPositions[4];
        };

        static RendererData* s_Data;
    };

} // namespace axiom