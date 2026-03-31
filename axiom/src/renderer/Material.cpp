#include "axiom/renderer/Material.h"
#include "axiom/renderer/Shader.h"
#include "axiom/renderer/Texture.h"
#include "axiom/core/Logger.h"

namespace axiom {

    // ---------------- Constructor ----------------
    Material::Material(std::shared_ptr<Shader> shader)
        : m_Shader(shader) {
    }

    // ---------------- Bind ----------------
    void Material::bind() {
        if (!m_Shader) {
            AXIOM_ERROR("Attempt to bind Material without a shader");
            return;
        }

        m_Shader->bind();

        // Textures binden
        for (auto& [name, binding] : m_Textures) {
            if (binding.texture) {
                uint64_t handle = binding.texture->getBindlessHandle();
                if (handle) {
                    // set bindless handle directly to the sampler uniform
                    m_Shader->setTextureHandle(name, handle);
                } else {
                    // fallback to regular binding - bind texture to slot and set uniform
                    binding.texture->bind(binding.slot);
                    m_Shader->setInt(name, (int)binding.slot);
                }
            } else {
                // no texture object (only slot set), ensure uniform points to slot
                m_Shader->setInt(name, (int)binding.slot);
            }
        }

        // Uniforms setzen
        for (auto& [name, value] : m_Ints)
            m_Shader->setInt(name, value);

        for (auto& [name, value] : m_Floats)
            m_Shader->setFloat(name, value);

        for (auto& [name, value] : m_Vec3s)
            m_Shader->setVec3(name, value);
    for (auto& [name, value] : m_Vec4s)
        m_Shader->setVec4(name, value);

        for (auto& [name, value] : m_Mat4s)
            m_Shader->setMat4(name, value);
    }

    // ---------------- Textures ----------------
    void Material::setTexture(const std::string& name, std::shared_ptr<Texture> texture, uint32_t slot) {
        m_Textures[name] = { texture, slot };
    }

    void Material::setTextureSlot(const std::string& name, uint32_t slot) {
        if (m_Textures.find(name) != m_Textures.end())
            m_Textures[name].slot = slot;
        else
            m_Textures[name] = { nullptr, slot };
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

    void Material::setVec4(const std::string& name, const glm::vec4& value) {
        m_Vec4s[name] = value;
    }

    void Material::setMat4(const std::string& name, const glm::mat4& value) {
        m_Mat4s[name] = value;
    }

    std::vector<std::tuple<std::string, std::shared_ptr<Texture>, uint32_t>> Material::getTextureBindings() const {
        std::vector<std::tuple<std::string, std::shared_ptr<Texture>, uint32_t>> out;
        for (auto& [name, binding] : m_Textures) {
            out.emplace_back(name, binding.texture, binding.slot);
        }
        return out;
    }

    std::shared_ptr<Shader> Material::getShader() const {
        return m_Shader;
    }

}