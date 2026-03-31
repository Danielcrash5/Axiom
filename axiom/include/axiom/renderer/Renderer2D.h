#pragma once
#include <glm/glm.hpp>
#include <memory>
#include <vector>

namespace axiom {
    class Camera;
    class VertexArray;
    class VertexBuffer;
    class IndexBuffer;
    class Shader;
    class Texture;
    class Material;

    /**
     * Simple 2D Renderer with clear, easy-to-understand architecture
     */
    class Renderer2D {
    public:
        static void Init();
        static void Shutdown();

        static void Begin(const Camera& camera);
        static void End();

        // Quad drawing
        static void DrawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color, int32_t depth = 0);
        static void DrawQuad(const glm::vec3& position, const glm::vec2& size, std::shared_ptr<Texture> texture, int32_t depth = 0);

        // Circle drawing
        static void DrawCircle(const glm::vec3& position, float radius, const glm::vec4& color, int32_t depth = 0);

        // Line drawing
        static void DrawLineStrip(const std::vector<glm::vec3>& points, const glm::vec4& color, float thickness = 0.02f, int32_t depth = 0);

        static std::shared_ptr<Shader> GetQuadShader();

    private:
        struct Vertex {
            glm::vec3 position;
            glm::vec2 uv;
            glm::vec4 color;
            float textureIndex;
        };

        struct RenderCommand {
            std::shared_ptr<Material> material;
            uint32_t vertexCount;
            uint32_t vertexOffset;
            glm::mat4 transform;
            std::shared_ptr<Texture> texture;
        };

        static constexpr uint32_t MAX_VERTICES = 100000;
        static constexpr uint32_t MAX_INDICES = 150000;
        static constexpr uint32_t MAX_TEXTURE_SLOTS = 32;

        static Vertex* s_VertexBuffer;
        static Vertex* s_VertexPtr;
        static uint32_t s_VertexCount;

        static std::shared_ptr<VertexArray> s_VAO;
        static std::shared_ptr<axiom::VertexBuffer> s_VBO;
        static std::shared_ptr<IndexBuffer> s_IBO;

        static std::shared_ptr<Shader> s_QuadShader;
        static std::shared_ptr<Material> s_DefaultMaterial;

        static std::vector<RenderCommand> s_Commands;
        static glm::mat4 s_ViewProjection;

        static std::shared_ptr<Texture> s_TextureSlots[MAX_TEXTURE_SLOTS];
        static uint32_t s_TextureSlotIndex;

        static int GetTextureSlot(std::shared_ptr<Texture> texture);
        static void ExecuteCommands();
    };
}
