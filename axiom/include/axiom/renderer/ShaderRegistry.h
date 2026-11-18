/**
 * @file axiom/include/axiom/renderer/ShaderRegistry.h
 * @brief Declares the public interface for the shader registry.
 */

#pragma once
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>
#include <axiom/renderer/rhi/ShaderDesc.h>
#include <axiom/renderer/rhi/RHITypes.h>
#include <axiom/renderer/ShaderCompiler.h>

namespace axiom::renderer {

/**
 * @brief Defines the available shader ID values.
 * @tparam ShaderID Template type used by this type.
 */
enum class ShaderID : uint64_t { invalid = 0 };

// Verwaltet ShaderIDs -> ShaderDesc. Shader werden offline zu SPIR-V
// kompiliert (glslc aus dem Vulkan SDK), diese Klasse laedt nur fertige
// .spv-Dateien - keine Runtime-Kompilierung (siehe Design-Doc Roadmap Phase 3).
/**
 * @brief Provides the shader registry functionality.
 * @tparam ShaderRegistry Template type used by this type.
 */
class ShaderRegistry {
public:
    /**
     * @brief Loads from files.
     * @param vertexSpvPath The vertex spv path used by this operation.
     * @param pixelSpvPath The pixel spv path used by this operation.
     * @param vertexLayout The vertex layout used by this operation.
     * @return The result of the operation.
     */
    [[nodiscard]] rhi::RHIResult<ShaderID> loadFromFiles(
        const std::string& vertexSpvPath, const std::string& pixelSpvPath,
        rhi::VertexLayout vertexLayout);

    // Wie loadFromFiles(), aber liest GLSL-QUELLTEXT (ueber VFS, z.B.
    // "engine://shaders/Quad.vert") und kompiliert ihn zur Laufzeit ueber
    // ShaderCompiler zu SPIR-V, statt eine fertige .spv-Datei zu laden.
    // Fuer den Editor gedacht (Live-Kompilierung beim Speichern); ein
    // spaeterer Packager kann dieselbe Methode fuer den Offline-Bake-
    // Schritt nutzen und das Ergebnis dann selbst als .spv wegschreiben
    // (ueber find()->vertexSpirv/pixelSpirv).
    /**
     * @brief Compiles from source.
     * @param vertexGlslPath The vertex glsl path used by this operation.
     * @param pixelGlslPath The pixel glsl path used by this operation.
     * @param vertexLayout The vertex layout used by this operation.
     * @return The result of the operation.
     */
    [[nodiscard]] rhi::RHIResult<ShaderID> compileFromSource(
        const std::string& vertexGlslPath, const std::string& pixelGlslPath,
        rhi::VertexLayout vertexLayout);

    /**
     * @brief Performs the find operation.
     * @param id The ID used by this operation.
     * @return The result of the operation.
     */
    [[nodiscard]] const rhi::ShaderDesc* find(ShaderID id) const;

private:
    /** @brief Stores the data for the entry. */
    struct Entry {
        /** @brief Stores the vertex code. */
        std::vector<uint32_t> vertexCode;
        /** @brief Stores the pixel code. */
        std::vector<uint32_t> pixelCode;
        /** @brief Stores the desc. */
        rhi::ShaderDesc desc; // Spans zeigen auf vertexCode/pixelCode oben
    };
    // unordered_map: Node-basiert, Adressen von Entry bleiben bei Rehash
    // stabil - wichtig, weil ShaderDesc-Spans auf Entry-eigene Vectors zeigen.
    /** @brief Stores the entries. */
    std::unordered_map<uint64_t, Entry> m_entries;
    /** @brief Stores the next ID. */
    uint64_t m_nextId = 1;

    /**
     * @brief Reads spirv file.
     * @param path The path used by this operation.
     * @return The result of the operation.
     */
    [[nodiscard]] static rhi::RHIResult<std::vector<uint32_t>> readSpirvFile(const std::string& path);
};

} // namespace axiom::renderer
