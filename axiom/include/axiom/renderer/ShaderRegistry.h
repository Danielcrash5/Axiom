#pragma once
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>
#include <axiom/renderer/rhi/ShaderDesc.h>
#include <axiom/renderer/rhi/RHITypes.h>

namespace axiom::renderer {

enum class ShaderID : uint64_t { Invalid = 0 };

// Verwaltet ShaderIDs -> ShaderDesc. Shader werden offline zu SPIR-V
// kompiliert (glslc aus dem Vulkan SDK), diese Klasse laedt nur fertige
// .spv-Dateien - keine Runtime-Kompilierung (siehe Design-Doc Roadmap Phase 3).
class ShaderRegistry {
public:
    [[nodiscard]] rhi::RHIResult<ShaderID> loadFromFiles(
        const std::string& vertexSpvPath, const std::string& pixelSpvPath,
        rhi::VertexLayout vertexLayout);

    [[nodiscard]] const rhi::ShaderDesc* find(ShaderID id) const;

private:
    struct Entry {
        std::vector<uint32_t> vertexCode;
        std::vector<uint32_t> pixelCode;
        rhi::ShaderDesc desc; // Spans zeigen auf vertexCode/pixelCode oben
    };
    // unordered_map: Node-basiert, Adressen von Entry bleiben bei Rehash
    // stabil - wichtig, weil ShaderDesc-Spans auf Entry-eigene Vectors zeigen.
    std::unordered_map<uint64_t, Entry> m_entries;
    uint64_t m_nextId = 1;

    [[nodiscard]] static rhi::RHIResult<std::vector<uint32_t>> readSpirvFile(const std::string& path);
};

} // namespace axiom::renderer
