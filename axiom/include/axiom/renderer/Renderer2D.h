// Renderer2D.h
#pragma once

#include <glm/glm.hpp>
#include <array>
#include <memory>
#include <vector>

namespace axiom {

    class Texture2D;
    class Material;
    class Shader;
    class VertexArray;
    class VertexBuffer;
    class IndexBuffer;
    class Sprite;

    struct SkinnedVertex2D {
        glm::vec3 Position;
        glm::vec4 Color;
        glm::vec2 TexCoord;
        glm::vec4 BoneIndices;
        glm::vec4 BoneWeights;
    };

    constexpr uint32_t MaxBones = 64;

    struct SkeletonPose2D {
        std::array<glm::mat4, MaxBones> BoneTransforms{};
        uint32_t BoneCount = 0;
    };

    struct SkinnedMesh2D {
        std::vector<SkinnedVertex2D> Vertices;
        std::vector<uint32_t> Indices;
        std::shared_ptr<Texture2D> Texture;
    };

    enum class LineCap {
        Butt = 0,
        Square = 1,
        Round = 2
    };

    enum class LineJoin {
        Miter = 0,
        Bevel = 1,
        Round = 2
    };

    class Renderer2D {
    public:
        static void Init();
        static void Shutdown();

        static void BeginScene();
        static void EndScene();

        // Quads
        static void DrawQuad(const glm::vec2& pos,
                             const glm::vec2& size,
                             const glm::vec4& color,
                             float z = 0.0f);

        static void DrawQuad(const glm::vec3& pos,
                             const glm::vec2& size,
                             const glm::vec4& color);

        static void DrawQuad(const glm::mat4& transform,
                             const glm::vec4& color);

        static void DrawQuad(const glm::vec2& pos,
                             const glm::vec2& size,
                             const std::shared_ptr<Texture2D>& texture,
                             float tiling = 1.0f,
                             const glm::vec4& tint = glm::vec4(1.0f),
                             float z = 0.0f);

        static void DrawQuad(const glm::vec3& pos,
                             const glm::vec2& size,
                             const std::shared_ptr<Texture2D>& texture,
                             float tiling = 1.0f,
                             const glm::vec4& tint = glm::vec4(1.0f));

        static void DrawQuad(const glm::mat4& transform,
                             const std::shared_ptr<Texture2D>& texture,
                             float tiling = 1.0f,
                             const glm::vec4& tint = glm::vec4(1.0f));

        static void DrawQuad(const glm::mat4& transform,
                             const std::shared_ptr<Texture2D>& texture,
                             const std::shared_ptr<Material>& materialOverride,
                             float tiling = 1.0f,
                             const glm::vec4& tint = glm::vec4(1.0f));

        // Sprites
        static void DrawSprite(const glm::mat4& transform,
                               const Sprite& sprite,
                               const glm::vec4& tint = glm::vec4(1.0f));

        static void DrawSprite(const glm::mat4& transform,
                               const Sprite& sprite,
                               const glm::vec4& tint,
                               const std::shared_ptr<Material>& materialOverride);

        // Skinned
        static void DrawSkinned(const glm::mat4& transform,
                                const SkinnedMesh2D& mesh,
                                const SkeletonPose2D& pose,
                                const glm::vec4& tint = glm::vec4(1.0f),
                                const std::shared_ptr<Material>& materialOverride = nullptr);

        // Lines
        static void DrawLine(const glm::vec3& p0,
                             const glm::vec3& p1,
                             const glm::vec4& color);

        static void DrawLine(const glm::vec3& p0,
                             const glm::vec3& p1,
                             const glm::vec4& color,
                             float thickness);

        static void DrawLine(const glm::vec3& p0,
                             const glm::vec3& p1,
                             const glm::vec4& color,
                             float thickness,
                             LineCap cap);

        static void DrawLineStrip(const std::vector<glm::vec3>& points,
                                  const glm::vec4& color,
                                  float thickness,
                                  bool closed = false,
                                  LineCap cap = LineCap::Butt,
                                  LineJoin join = LineJoin::Miter,
                                  float miterLimit = 4.0f);

        // Rects
        static void DrawRect(const glm::vec3& pos,
                             const glm::vec2& size,
                             const glm::vec4& color,
                             float z = 0.0f,
                             float thickness = 1.5f);

        static void DrawRect(const glm::mat4& transform,
                             const glm::vec4& color,
                             float thickness = 1.5f);

        // Circles
        static void DrawCircle(const glm::vec2& pos,
                               float radius,
                               float thickness,
                               const glm::vec4& color,
                               float z = 0.0f);

        static void DrawCircle(const glm::vec3& pos,
                               float radius,
                               float thickness,
                               const glm::vec4& color);

        static void DrawCircle(const glm::mat4& transform,
                               float thickness,
                               const glm::vec4& color);

    private:
        struct QuadVertex {
            glm::vec3 Position;
            glm::vec4 Color;
            glm::vec2 TexCoord;
            float TexIndex;
            float TilingFactor;
        };

        struct CircleVertex {
            glm::vec3 Position;
            glm::vec3 LocalPosition;
            glm::vec4 Color;
            float Thickness;
        };

        struct RendererData {
            static constexpr uint32_t MaxQuads = 20000;
            static constexpr uint32_t MaxVertices = MaxQuads * 4;
            static constexpr uint32_t MaxIndices = MaxQuads * 6;
            static constexpr uint32_t MaxTextureSlots = 32;

            std::shared_ptr<VertexArray> QuadVAO;
            std::shared_ptr<VertexBuffer> QuadVBO;
            std::shared_ptr<IndexBuffer> QuadIBO;

            QuadVertex* QuadBufferBase = nullptr;
            QuadVertex* QuadBufferPtr = nullptr;

            std::shared_ptr<Texture2D> TextureSlots[MaxTextureSlots];
            uint32_t TextureSlotIndex = 1;

            glm::vec4 QuadVertexPositions[4];

            std::shared_ptr<Material> QuadMaterial;

            // Circles
            std::shared_ptr<VertexArray> CircleVAO;
            std::shared_ptr<VertexBuffer> CircleVBO;
            CircleVertex* CircleBufferBase = nullptr;
            CircleVertex* CircleBufferPtr = nullptr;
            std::shared_ptr<Material> CircleMaterial;

            // Skinned
            std::shared_ptr<VertexArray> SkinnedVAO;
            std::shared_ptr<VertexBuffer> SkinnedVBO;
            std::shared_ptr<IndexBuffer> SkinnedIBO;
            std::shared_ptr<Material> SkinnedMaterial;
        };

        static RendererData* s_Data;

        static void StartBatch();
        static void FlushAll();

        static void FlushQuads();
        static void FlushCircles();

        static void EnsureQuadCapacity(uint32_t quadCount = 1);
        static void EnsureCircleCapacity(uint32_t circleCount = 1);

        static float AcquireTextureSlot(const std::shared_ptr<Texture2D>& texture);
        static void Configure2DMaterial(const std::shared_ptr<Material>& material);
    };

} // namespace axiom
