#pragma once
#include <glm/glm.hpp>
#include "Model.h"
#include "RenderCommand.h"

namespace axiom {
	struct ClearState {
		bool ClearColor = true;
		bool ClearDepth = true;

		glm::vec4 Color = { 0.1f, 0.1f, 0.1f, 1.0f };
		float Depth = 1.0f;
	};

	struct SceneData {
		glm::mat4 ViewProjection;
	};

	class Renderer {
	public:
		static void Init();

		static void BeginScene(const glm::mat4& ViewProjection, ClearState clearState);

		static void EndScene();
		static const glm::mat4& GetViewProjection();

		static void Submit(const std::shared_ptr<Model>& model, const glm::mat4& transform);
		static void Submit(
			const std::shared_ptr<VertexArray>& vao,
			const std::shared_ptr<Material>& material,
			uint32_t indexCount,
			const glm::mat4& transform
		);
		static void SubmitLines(
			const std::shared_ptr<VertexArray>& vao,
			const std::shared_ptr<Material>& material,
			uint32_t indexCount
		);

	private:
		static SceneData s_SceneData;
	};

}
