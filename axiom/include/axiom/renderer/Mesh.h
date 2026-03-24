#pragma once

#include <memory>
#include <vector>

#include "axiom/renderer/VertexArray.h"
#include "axiom/renderer/Material.h"

namespace axiom {

    // ===================== SUBMESH =====================

    struct Submesh {
        uint32_t IndexOffset = 0;
        uint32_t IndexCount = 0;

        std::shared_ptr<Material> Material;
    };

    // ===================== MESH =====================

    class Mesh {
    public:

        Mesh(const std::shared_ptr<VertexArray>& vao);

        void AddSubmesh(const Submesh& submesh);

        void Draw() const;

    private:

        std::shared_ptr<VertexArray> m_VertexArray;
        std::vector<Submesh> m_Submeshes;
    };

}