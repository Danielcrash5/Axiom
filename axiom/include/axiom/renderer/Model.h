#pragma once
#include <vector>
#include <memory>
#include "Mesh.h"
#include "Material.h"

namespace axiom {

    class Model {
    public:
        void AddMesh(const std::shared_ptr<Mesh>& mesh) {
            m_Meshes.push_back(mesh);
        }
        void AddMaterial(const std::shared_ptr<Material>& material) {
            m_Materials.push_back(material);
        }

        const std::vector<std::shared_ptr<Mesh>>& GetMeshes() const {
            return m_Meshes;
        }
        const std::vector<std::shared_ptr<Material>>& GetMaterials() const {
            return m_Materials;
        }

    private:
        std::vector<std::shared_ptr<Mesh>> m_Meshes;
        std::vector<std::shared_ptr<Material>> m_Materials;
    };

}