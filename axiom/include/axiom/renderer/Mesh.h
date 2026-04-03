#pragma once
#include <vector>
#include <memory>
#include "Submesh.h"
#include "VertexArray.h"

namespace axiom {

    class Mesh {
    public:
        Mesh(const std::shared_ptr<VertexArray>& vao) : m_VertexArray(vao) {
        }

        void AddSubmesh(const Submesh& submesh) {
            m_Submeshes.push_back(submesh);
        }

        const std::vector<Submesh>& GetSubmeshes() const {
            return m_Submeshes;
        }

        const std::shared_ptr<VertexArray>& GetVertexArray() const {
            return m_VertexArray;
        }

    private:
        std::shared_ptr<VertexArray> m_VertexArray;
        std::vector<Submesh> m_Submeshes;
    };

}