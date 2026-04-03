#include "axiom/renderer/Renderer.h"

namespace axiom {
        void Renderer::BeginScene(const Camera& camera) {
            s_SceneData.ViewProjection = camera.GetViewProjection();
        }

        void Renderer::EndScene() {
            // Optional: Flush Commands, Multithreading später
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

                    RenderCommand::SetRenderState(material->GetRenderState());

                    RenderCommand::DrawIndexed(mesh->GetVertexArray(), submesh.IndexCount, submesh.IndexOffset);
                }
            }
        }

    SceneData Renderer::s_SceneData;

}