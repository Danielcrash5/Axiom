#include "axiom/renderer/Material.h"
#include "axiom/renderer/Shader.h"
#include "axiom/renderer/Texture2D.h"
#include "axiom/renderer/RenderCommand.h"
#include <stdexcept>

namespace axiom {

	Material::Material(const std::shared_ptr<Shader>& shader)
		: m_Shader(shader) {
	}

	void Material::Set(const std::string& name, const UniformValue& value) {
		m_Uniforms[name] = value;
	}

	void Material::SetTexture(const std::string& name, const std::shared_ptr<Texture2D>& texture) {
		m_Textures[name] = texture;
	}

	void Material::SetRenderState(const RenderState& state) {
		m_RenderState = state;
	}

	RenderState& Material::GetRenderState() {
		return m_RenderState;
	}

	void Material::Bind() {
		if (!m_Shader)
			throw std::runtime_error("Material::Bind called without a valid shader");

		m_Shader->Bind();

		// --- 1. Uniforms ---
		for (auto& [name, value] : m_Uniforms) {
			std::visit([&](auto&& v) {
				using T = std::decay_t<decltype(v)>;

				if constexpr (std::is_same_v<T, int>)
					m_Shader->SetUniform1i(name, v);
				else if constexpr (std::is_same_v<T, float>)
					m_Shader->SetUniform1f(name, v);
				else if constexpr (std::is_same_v<T, glm::vec2>)
					m_Shader->SetUniform2f(name, v);
				else if constexpr (std::is_same_v<T, glm::vec3>)
					m_Shader->SetUniform3f(name, v);
				else if constexpr (std::is_same_v<T, glm::vec4>)
					m_Shader->SetUniform4f(name, v);
				else if constexpr (std::is_same_v<T, glm::mat3>)
					m_Shader->SetUniformMat3(name, v);
				else if constexpr (std::is_same_v<T, glm::mat4>)
					m_Shader->SetUniformMat4(name, v);
					   }, value);
		}

		// --- 2. Textures ---
		uint32_t slot = 0;

		for (auto& [name, texture] : m_Textures) {
			if (!texture)
				continue;

			// Shader entscheidet selbst: Bindless oder Slot
			m_Shader->SetTexture(name, texture, slot++);
		}

		// --- 3. RenderState ---
		RenderCommand::SetRenderState(m_RenderState);
	}

} // namespace axiom
