#include "axiom/renderer/Material.h"
#include "axiom/renderer/Shader.h"
#include "axiom/renderer/Texture.h"

namespace axiom {

    // ---------------- Constructor ----------------
    Material::Material(std::shared_ptr<Shader> shader)
        : m_Shader(shader) {
    }

    // ---------------- Bind ----------------
    void Material::bind() {
        m_Shader->bind();

        // Textures binden
        for (auto& [name, binding] : m_Textures) {
            binding.texture->bind(binding.slot);
            m_Shader->setInt(name, binding.slot);
        }

        // Uniforms setzen
        for (auto& [name, value] : m_Ints)
            m_Shader->setInt(name, value);

        for (auto& [name, value] : m_Floats)
            m_Shader->setFloat(name, value);

        for (auto& [name, value] : m_Vec3s)
            m_Shader->setVec3(name, value);

        for (auto& [name, value] : m_Mat4s)
            m_Shader->setMat4(name, value);
    }

    // ---------------- Textures ----------------
    void Material::setTexture(const std::string& name, std::shared_ptr<Texture> texture, uint32_t slot) {
        m_Textures[name] = { texture, slot };
    }

    // ---------------- Uniforms ----------------
    void Material::setInt(const std::string& name, int value) {
        m_Ints[name] = value;
    }

    void Material::setFloat(const std::string& name, float value) {
        m_Floats[name] = value;
    }

    void Material::setVec3(const std::string& name, const glm::vec3& value) {
        m_Vec3s[name] = value;
    }

    void Material::setMat4(const std::string& name, const glm::mat4& value) {
        m_Mat4s[name] = value;
    }

}