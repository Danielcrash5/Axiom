#pragma once
#include <glm/glm.hpp>
#include "Camera.h"
#include "Model.h"
#include "RenderCommand.h"

namespace axiom {

	struct SceneData {
		glm::mat4 ViewProjection;
	};

	class Renderer {
	public:
		static void BeginScene(const Camera& camera);

		static void EndScene();

		static void Submit(const std::shared_ptr<Model>& model, const glm::mat4& transform);

	private:
		static SceneData s_SceneData;
	};

}