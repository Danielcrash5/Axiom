#pragma once

#include <unordered_map>
#include <string>
#include <memory>
#include <variant>

#include "axiom/renderer/Shader.h"
#include "axiom/renderer/Texture.h"
#include <glm/glm.hpp>

namespace axiom {

    class Texture;

    using MaterialValue = std::variant<
        float,
        int,
        glm::vec2,
        glm::vec3,
        glm::vec4,
        glm::mat4
    >;

    class Material {
    public:

        Material(const std::shared_ptr<Shader>& shader);

        void Set(const std::string& name, float value);
        void Set(const std::string& name, int value);
        void Set(const std::string& name, std::shared_ptr<Texture> texture);
        void Set(const std::string& name, const glm::mat4& value);

        void Bind() const;

    private:

        std::shared_ptr<Shader> m_Shader;

        std::unordered_map<std::string, MaterialValue> m_Values;
        std::unordered_map<std::string, std::shared_ptr<Texture>> m_Textures;

    };

}