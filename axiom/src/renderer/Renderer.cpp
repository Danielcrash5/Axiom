#include "axiom/renderer/Renderer.h"

namespace axiom {
	void Renderer::Init() {
		RenderCommand::Init();
	}

	void Renderer::BeginScene(const std::shared_ptr<Camera>& camera, ClearState clearState) {
		if (clearState.ClearColor)
			RenderCommand::SetClearColor(clearState.Color);


		s_SceneData.ViewProjection = camera->GetViewProjection();
		RenderCommand::Clear();
	}

	void Renderer::EndScene() {
		// Optional: Flush Commands, Multithreading später
	}

	const glm::mat4& Renderer::GetViewProjection() {
		return s_SceneData.ViewProjection;
	}

	void Renderer::Submit(const std::shared_ptr<Model>& model, const glm::mat4& transform) {
		auto& meshes = model->GetMeshes();
		auto& materials = model->GetMaterials();

		for (size_t i = 0; i < meshes.size(); i++) {
			auto mesh = meshes[i];
			for (auto& submesh : mesh->GetSubmeshes()) {
				auto material = materials[submesh.MaterialIndex];

				material->Set("u_ViewProjection", s_SceneData.ViewProjection);
				material->Set("u_Transform", transform);
				material->Bind();

				RenderCommand::DrawIndexed(mesh->GetVertexArray(), submesh.IndexCount, submesh.IndexOffset);
			}
		}
	}

	void Renderer::Submit(
		const std::shared_ptr<VertexArray>& vao,
		const std::shared_ptr<Material>& material,
		uint32_t indexCount,
		const glm::mat4& transform
	) {
		material->Set("u_ViewProjection", s_SceneData.ViewProjection);
		material->Set("u_Transform", transform);
		material->Bind();

		RenderCommand::DrawIndexed(vao, indexCount);
	}

	void Renderer::SubmitLines(const std::shared_ptr<VertexArray>& vao,
							   const std::shared_ptr<Material>& material,
							   uint32_t indexCount) {
		material->Set("u_ViewProjection", s_SceneData.ViewProjection);
		material->Bind();
		RenderCommand::DrawLinesIndexed(vao, indexCount);
	}

	SceneData Renderer::s_SceneData;

}
