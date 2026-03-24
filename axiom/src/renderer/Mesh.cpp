#include "axiom/renderer/Mesh.h"
#include "axiom/renderer/Renderer.h"

namespace axiom {

    // ===================== CONSTRUCTOR =====================

    Mesh::Mesh(const std::shared_ptr<VertexArray>& vao)
        : m_VertexArray(vao) {
    }

    // ===================== ADD SUBMESH =====================

    void Mesh::AddSubmesh(const Submesh& submesh) {
        m_Submeshes.push_back(submesh);
    }

    // ===================== DRAW =====================

    void Mesh::Draw() const {
        for (const auto& submesh : m_Submeshes) {
            if (submesh.Material)
                submesh.Material->Bind();

            Renderer::DrawIndexed(
                m_VertexArray,
                submesh.IndexCount,
                submesh.IndexOffset
            );
        }
    }

}