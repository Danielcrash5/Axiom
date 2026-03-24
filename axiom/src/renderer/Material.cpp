#include "axiom/renderer/Material.h"
#include "axiom/platform/opengl/OpenGLShader.h"
#include "axiom/core/Logger.h"

#include <glad/glad.h>

namespace axiom {

    Material::Material(const std::shared_ptr<Shader>& shader)
        : m_Shader(shader) {
    }

    // ===================== SET =====================

    void Material::Set(const std::string& name, float value) {
        m_Values[name] = value;
    }

    void Material::Set(const std::string& name, int value) {
        m_Values[name] = value;
    }

    /*void Material::Set(const std::string& name, std::shared_ptr<Texture> texture) {
        m_Textures[name] = texture;
    }*/

    void Material::Set(const std::string& name, const glm::mat4& value) {
        m_Values[name] = value;
    }

    // ===================== BIND =====================

    void Material::Bind() const {
        m_Shader->Bind();

        auto glShader = std::dynamic_pointer_cast<OpenGLShader>(m_Shader);
        uint32_t program = glShader->GetRendererID();

        uint32_t textureSlot = 0;

        for (const auto& res : m_Shader->GetResources()) {
            const std::string& name = res.name;

            // TEXTURE
            if (res.type == ShaderResourceType::Sampler2D) {
                auto it = m_Textures.find(name);
                if (it == m_Textures.end())
                    continue;

                glActiveTexture(GL_TEXTURE0 + textureSlot);

                // TODO: texture->Bind()
                // it->second->Bind();

                glUniform1i(res.location, textureSlot);

                textureSlot++;
            }

            // VALUES
            else if (res.type == ShaderResourceType::Uniform) {
                auto it = m_Values.find(name);
                if (it == m_Values.end())
                    continue;

                if (std::holds_alternative<float>(it->second)) {
                    glUniform1f(res.location, std::get<float>(it->second));
                } else if (std::holds_alternative<int>(it->second)) {
                    glUniform1i(res.location, std::get<int>(it->second));
                }
                else if (std::holds_alternative<glm::mat4>(it->second)) {
                    glUniformMatrix4fv(
                        res.location,
                        1,
                        GL_FALSE,
                        &std::get<glm::mat4>(it->second)[0][0]
                    );
                }
            }

            
        }
    }

}