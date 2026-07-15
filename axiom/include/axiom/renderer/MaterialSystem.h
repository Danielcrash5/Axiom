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
class MaterialSystem {
public:
    MaterialSystem(rhi::IRHIBackend& backend, ShaderRegistry& shaders)
        : m_backend(backend), m_shaders(shaders) {}

    // bindGroupLayoutDesc beschreibt die Bindings, die material.params
    // erwartet - der Aufrufer deklariert das explizit (kein Shader-Reflection
    // in Phase 3). BindlessIndex-Parameter werden NICHT Teil dieser
    // materialeigenen BindGroup - die laufen ueber ein zusaetzliches Set,
    // das separat aus BindlessTextureHeap::bindGroup() kommt.
    [[nodiscard]] rhi::RHIResult<void> build(
        Material& material,
        const rhi::BindGroupLayoutDesc& bindGroupLayoutDesc,
        rhi::TextureFormat colorTargetFormat,
        std::optional<rhi::TextureFormat> depthTargetFormat = std::nullopt);

private:
    rhi::IRHIBackend& m_backend;
    ShaderRegistry& m_shaders;
};

} // namespace axiom::renderer
