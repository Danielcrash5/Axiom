#pragma once
#include <vector>
#include <glm/glm.hpp>
#include "Camera.h"
#include "Material.h"

namespace axiom {

    
    class VertexArray;
    class VertexBuffer;
    class IndexBuffer;
    class Shader;
    class Texture;

    class Renderer2D {
    public:
        static void Init();
        static void Shutdown();

        static void Begin(const Camera& camera);
        static void End();

        static void DrawQuad(const glm::mat4& transform, std::shared_ptr<Material> material);
        // Helper overloads
        static void DrawQuad(const glm::vec3& position, const glm::vec2& size, float rotation = 0.0f, std::shared_ptr<Material> material = nullptr);
        static void DrawQuad(const glm::vec3& position, const glm::vec2& size, std::shared_ptr<Texture> texture);
        static void DrawQuadOutline(const glm::vec3& position, const glm::vec2& size, float rotation = 0.0f, std::shared_ptr<Material> material = nullptr, float thickness = 0.02f);

        static void DrawCircle(const glm::mat4& transform, std::shared_ptr<Material> material);
        // Helper overloads
        static void DrawCircle(const glm::vec3& position, float radius, std::shared_ptr<Material> material = nullptr);

        enum class LineJoin {
            Miter,
            Bevel,
            Round
        };

        enum class LineCap {
            Butt,
            Square,
            Round
        };

        static void DrawLine(const glm::vec3& p0, const glm::vec3& p1, std::shared_ptr<Material> material = nullptr, float thickness = 0.02f, LineCap cap = LineCap::Round);
        static void DrawLineStrip(const std::vector<glm::vec3>& points, std::shared_ptr<Material> material = nullptr, float thickness = 0.02f, LineJoin join = LineJoin::Round, LineCap cap = LineCap::Round);

    private:

        struct QuadVertex {
            glm::vec3 Position;
            glm::vec2 UV;
            glm::vec4 Color;
            float TexIndex;
        };

        static constexpr uint32_t MaxQuads = 10000;
        static constexpr uint32_t MaxVertices = MaxQuads * 4;
        static constexpr uint32_t MaxIndices = MaxQuads * 6;

        static QuadVertex* s_VertexBufferBase;
        static QuadVertex* s_VertexBufferPtr;

        static uint32_t s_IndexCount;

        static std::shared_ptr<VertexArray> s_QuadVAO;
        static std::shared_ptr<VertexBuffer> s_VertexBuffer;
        static std::shared_ptr<IndexBuffer> s_IndexBuffer;

        // Default Materials
        static std::shared_ptr<Material> s_DefaultQuadMaterial;
        static std::shared_ptr<Material> s_DefaultCircleMaterial;
        static std::shared_ptr<Material> s_DefaultLineMaterial;
        
        struct RenderItem {
            glm::mat4 Transform;
            std::shared_ptr<Material> Material;
            uint32_t Type; // 0=quad,1=circle,2=line
            std::vector<glm::vec3> Points; // for lines/strips
            float Thickness = 0.02f;
            glm::vec4 Color = glm::vec4(1.0f);
        };

        // Max texture slots supported by the batch (GL spec guarantees at least 16)
        static constexpr uint32_t MaxTextureSlots = 32;
        static std::shared_ptr<Texture> s_TextureSlots[MaxTextureSlots];

        // Assign a texture to a slot for the current batch. Returns slot index or -1 on failure.
        static int bindTextureToSlot(std::shared_ptr<Texture> texture);

        static std::vector<RenderItem> s_Items;
        static glm::mat4 s_ViewProjection;
    };

}