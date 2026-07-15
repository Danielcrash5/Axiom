#include <axiom/renderer/ShaderRegistry.h>
#include <fstream>

namespace axiom::renderer {

rhi::RHIResult<std::vector<uint32_t>> ShaderRegistry::readSpirvFile(const std::string& path) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file) {
        return std::unexpected(rhi::RHIError::InvalidDescriptor);
    }

    auto sizeBytes = static_cast<size_t>(file.tellg());
    if (sizeBytes == 0 || sizeBytes % sizeof(uint32_t) != 0) {
        // SPIR-V ist 32-bit-wortweise aligned - falsche Groesse heisst
        // fast immer "falsche/kaputte Datei", nicht "valides, aber krummes SPIR-V".
        return std::unexpected(rhi::RHIError::InvalidDescriptor);
    }
    file.seekg(0);

    std::vector<uint32_t> buffer(sizeBytes / sizeof(uint32_t));
    if (!file.read(reinterpret_cast<char*>(buffer.data()), static_cast<std::streamsize>(sizeBytes))) {
        return std::unexpected(rhi::RHIError::Unknown);
    }
    return buffer;
}

rhi::RHIResult<ShaderID> ShaderRegistry::loadFromFiles(
    const std::string& vertexSpvPath, const std::string& pixelSpvPath,
    rhi::VertexLayout vertexLayout) {
    auto vertexCode = readSpirvFile(vertexSpvPath);
    if (!vertexCode) return std::unexpected(vertexCode.error());

    auto pixelCode = readSpirvFile(pixelSpvPath);
    if (!pixelCode) return std::unexpected(pixelCode.error());

    uint64_t id = m_nextId++;
    Entry& entry = m_entries[id]; // erzeugt Node an fester, stabiler Adresse

    entry.vertexCode = std::move(*vertexCode);
    entry.pixelCode = std::move(*pixelCode);
    entry.desc.stages = rhi::ShaderStage::Vertex | rhi::ShaderStage::Pixel;
    entry.desc.vertexSpirv = entry.vertexCode;
    entry.desc.pixelSpirv = entry.pixelCode;
    entry.desc.vertexLayout = std::move(vertexLayout);

    return static_cast<ShaderID>(id);
}

const rhi::ShaderDesc* ShaderRegistry::find(ShaderID id) const {
    auto it = m_entries.find(static_cast<uint64_t>(id));
    if (it == m_entries.end()) return nullptr;
    return &it->second.desc;
}

} // namespace axiom::renderer
