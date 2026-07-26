#include <axiom/renderer/ShaderRegistry.h>
#include <axiom/assets/VFS.h>
#include <cstring>

namespace axiom::renderer {

rhi::RHIResult<std::vector<uint32_t>> ShaderRegistry::readSpirvFile(const std::string& path) {
    std::vector<uint8_t> rawBytes;
    if (!axiom::VFS::ReadFile(path, rawBytes)) {
        return std::unexpected(rhi::RHIError::InvalidDescriptor);
    }

    if (rawBytes.empty() || rawBytes.size() % sizeof(uint32_t) != 0) {
        // SPIR-V ist 32-bit-wortweise aligned - falsche Groesse heisst
        // fast immer "falsche/kaputte Datei", nicht "valides, aber krummes SPIR-V".
        return std::unexpected(rhi::RHIError::InvalidDescriptor);
    }

    // memcpy statt reinterpret_cast<uint32_t*>(rawBytes.data()): vector<uint8_t>
    // garantiert keine 4-Byte-Alignment fuer seinen Buffer - direktes Reinterpretieren
    // waere UB. memcpy ist der sichere, portable Weg, die Bytes umzukopieren.
    std::vector<uint32_t> buffer(rawBytes.size() / sizeof(uint32_t));
    std::memcpy(buffer.data(), rawBytes.data(), rawBytes.size());
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
