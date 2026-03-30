#pragma once
#include <memory>
#include <string>
#include <unordered_map>
#include <glm/glm.hpp>

namespace axiom {

    class Shader;
    class Texture;

    class Material {
    public:
        Material(std::shared_ptr<Shader> shader);

        void bind();

        // Textures
        void setTexture(const std::string& name, std::shared_ptr<Texture> texture, uint32_t slot);

        // Uniforms
        void setInt(const std::string& name, int value);
        void setFloat(const std::string& name, float value);
        void setVec3(const std::string& name, const glm::vec3& value);
        void setMat4(const std::string& name, const glm::mat4& value);

    private:
        std::shared_ptr<Shader> m_Shader;

        struct TextureBinding {
            std::shared_ptr<Texture> texture;
            uint32_t slot;
        };

        std::unordered_map<std::string, TextureBinding> m_Textures;

        // einfache Uniform Cache (optional später verbessern)
        std::unordered_map<std::string, int> m_Ints;
        std::unordered_map<std::string, float> m_Floats;
        std::unordered_map<std::string, glm::vec3> m_Vec3s;
        std::unordered_map<std::string, glm::mat4> m_Mat4s;
    };

}