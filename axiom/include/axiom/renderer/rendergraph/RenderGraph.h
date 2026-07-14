#pragma once
#include "RenderPass.h"
#include <axiom/renderer/rhi/IRHIBackend.h>
#include <memory>
#include <unordered_map>
#include <vector>

namespace axiom::renderer::rendergraph {

class RenderGraph {
  public:
    explicit RenderGraph(rhi::IRHIBackend &backend) : m_backend(backend) {}

    void addPass(std::unique_ptr<RenderPass> pass);

    // Löst NUR Dependencies/Layouts auf (welche Barriers wo nötig sind),
    // erzeugt transiente Resourcen. Keine Szenen-/Sortier-/Batch-Logik
    // (die liegt in RenderQueueSystem/BatchBuilder, Abschnitt 10).
    [[nodiscard]] rhi::RHIResult<void> compile();

    // Führt alle Passes in der von compile() bestimmten Reihenfolge aus.
    [[nodiscard]] rhi::RHIResult<void> execute();

  private:
    friend class RenderGraphBuilder;
    friend class RenderContext;

    struct ResourceEntry {
        ResourceType type = ResourceType::Texture;
        rhi::TextureHandle textureHandle; // gültig bei Texture
        rhi::TextureLayout currentLayout = rhi::TextureLayout::Undefined;
        bool isImported = false; // false = transient, vom Graph selbst erzeugt
        TextureResourceDesc desc;
        uint32_t generation = 0;
    };

    struct ResourceAccess {
        ResourceHandle handle;
        AccessType access;
    };

    struct PassEntry {
        std::unique_ptr<RenderPass> pass;
        std::vector<ResourceAccess> accesses;
    };

    // Von RenderGraphBuilder genutzt:
    ResourceHandle registerTransientTexture(const TextureResourceDesc &desc);
    ResourceHandle registerImportedTexture(rhi::TextureHandle handle,
                                           const TextureResourceDesc &desc);
    void recordAccess(uint32_t passIndex, ResourceHandle handle,
                      AccessType access);

    // Von RenderContext genutzt:
    [[nodiscard]] rhi::TextureHandle
    resolveTexture(ResourceHandle handle) const;

    // Bestimmt für ein AccessType das benötigte Layout. Fürs Clear-Test-Pass
    // reicht TransferDst; ColorAttachment/ShaderReadOnly kommen mit
    // Pipelines/Sampling in Phase 3.
    [[nodiscard]] rhi::TextureLayout requiredLayoutFor(AccessType access) const;

    rhi::IRHIBackend &m_backend;
    std::vector<PassEntry> m_passes;
    std::vector<ResourceEntry> m_resources;
};

} // namespace axiom::renderer::rendergraph