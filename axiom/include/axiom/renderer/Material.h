#pragma once
#include <unordered_map>
#include <variant>
#include <memory>
#include <string>
#include <glm/glm.hpp>

#include "RenderState.h"

namespace axiom {

    class Shader;
    class Texture2D;

    class Material {
    public:
        using UniformValue = std::variant<
            int,
            float,
            glm::vec2,
            glm::vec3,
            glm::vec4,
            glm::mat3,
            glm::mat4
        >;

        Material(const std::shared_ptr<Shader>& shader);

        // Uniforms
        void Set(const std::string& name, const UniformValue& value);

        // Textures
        void SetTexture(const std::string& name, const std::shared_ptr<Texture2D>& texture);

        // RenderState
        void SetRenderState(const RenderState& state);
        RenderState& GetRenderState();

        void Bind();

        std::shared_ptr<Shader> GetShader() const {
            return m_Shader;
        }

    private:
        std::shared_ptr<Shader> m_Shader;

        std::unordered_map<std::string, UniformValue> m_Uniforms;
        std::unordered_map<std::string, std::shared_ptr<Texture2D>> m_Textures;

        RenderState m_RenderState;
    };

} // namespace axiom