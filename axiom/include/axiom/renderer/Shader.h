#pragma once
#include <string>
#include <memory>
#include <unordered_set>
#include <glm/glm.hpp>
#include "Texture2D.h"
namespace axiom {
	class Shader {
	public:
		virtual ~Shader() = default;

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		// Uniforms
		virtual void SetUniform1i(const std::string& name, int value) = 0;
		virtual void SetUniform1f(const std::string& name, float value) = 0;
		virtual void SetUniform2f(const std::string& name, const glm::vec2& value) = 0;
		virtual void SetUniform3f(const std::string& name, const glm::vec3& value) = 0;
		virtual void SetUniform4f(const std::string& name, const glm::vec4& value) = 0;
		virtual void SetUniformMat3(const std::string& name, const glm::mat3& value) = 0;
		virtual void SetUniformMat4(const std::string& name, const glm::mat4& value) = 0;
		virtual void SetTexture(const std::string& name, const std::shared_ptr<axiom::Texture2D>& texture, uint32_t slot) = 0;
		virtual bool HasUniform(const std::string& name) const = 0;

		static std::shared_ptr<Shader> Create(
			const std::string& path,
			const std::unordered_set<std::string>& defines = {}
		);

		static std::shared_ptr<Shader> CreateFromMemory(
			const std::string& source,
			const std::unordered_set<std::string>& defines = {}
		);

	};
}
