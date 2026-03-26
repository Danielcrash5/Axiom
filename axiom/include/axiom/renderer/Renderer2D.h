#pragma once

#include <memory>
#include <glm/glm.hpp>

namespace axiom {

    class Texture;
    class Material;
    class VertexArray;
    class VertexBuffer;
    class Shader;

    class Renderer2D {
    public:

        static void Init();
        static void Shutdown();

        static void Begin(const glm::mat4& viewProj);
        static void End();

        static void DrawQuad(
            const glm::vec3& position,
            const glm::vec2& size,
            const glm::vec4& color
        );

        static void DrawQuad(
            const glm::vec3& position,
            const glm::vec2& size,
            const std::shared_ptr<Texture>& texture
        );

        static void DrawQuad(
            const glm::vec3& position,
            const glm::vec2& size,
            const float rotation,
            const std::shared_ptr<Texture>& texture
        );

        static void DrawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color, float rotation);

        static void DrawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color, const std::shared_ptr<Texture>& texture);

        static void DrawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color, float rotation, const std::shared_ptr<Texture>& texture);

    private:

        static void DrawQuadInternal(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color, float rotation, const std::shared_ptr<Texture>& texture);

        static void Flush();
        static void StartBatch();

    };

}
