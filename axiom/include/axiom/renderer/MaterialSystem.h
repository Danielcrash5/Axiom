/**
 * @file axiom/include/axiom/renderer/MaterialSystem.h
 * @brief Declares the public interface for the material system.
 */

#pragma once
#include <optional>
#include <axiom/renderer/rhi/IRHIBackend.h>
#include <axiom/renderer/Material.h>
#include <axiom/renderer/ShaderRegistry.h>

namespace axiom::renderer {

// Baut aus einem Material (shaderId + state + params) die tatsaechlichen
// GPU-Objekte (Pipeline, BindGroupLayout, BindGroup) und schreibt sie ueber
// friend-Zugriff in Material zurueck. Getrennt von Material selbst, damit
// Material eine reine Datenklasse bleiben kann.
/**
 * @brief Manages the material resources used for rendering.
 * @tparam MaterialSystem Template type used by this type.
 */
class MaterialSystem {
public:
    /**
     * @brief Creates an instance of material system.
     * @param backend The backend used by this operation.
     * @param shaders The shaders used by this operation.
     */
    MaterialSystem(rhi::IRHIBackend& backend, ShaderRegistry& shaders)
        : m_backend(backend), m_shaders(shaders) {}

    // bindGroupLayoutDesc beschreibt die Bindings, die material.params
    // erwartet - der Aufrufer deklariert das explizit (kein Shader-Reflection
    // in Phase 3). BindlessIndex-Parameter werden NICHT Teil dieser
    // materialeigenen BindGroup - die laufen ueber ein zusaetzliches Set,
    // das separat aus BindlessTextureHeap::bindGroup() kommt.
    /**
     * @brief Builds the requested resource.
     * @param material The material used by this operation.
     * @param bindGroupLayoutDesc The bind group layout desc used by this operation.
     * @param colorTargetFormat The color target format used by this operation.
     * @param depthTargetFormat The depth target format used by this operation.
     */
    [[nodiscard]] rhi::RHIResult<void> build(
        Material& material,
        const rhi::BindGroupLayoutDesc& bindGroupLayoutDesc,
        rhi::TextureFormat colorTargetFormat,
        std::optional<rhi::TextureFormat> depthTargetFormat = std::nullopt);

private:
    /** @brief Stores the backend. */
    rhi::IRHIBackend& m_backend;
    /** @brief Stores the shaders. */
    ShaderRegistry& m_shaders;
};

} // namespace axiom::renderer
