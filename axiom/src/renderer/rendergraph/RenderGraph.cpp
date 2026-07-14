#include <axiom/renderer/rendergraph/RenderGraph.h>

namespace axiom::renderer::rendergraph {

void RenderGraph::addPass(std::unique_ptr<RenderPass> pass) {
    // Bewusst NUR speichern – setup() wird erst in compile() aufgerufen,
    // damit der Graph erst alle Passes vollständig kennt, bevor er
    // irgendetwas anlegt oder Entscheidungen trifft.
    m_passes.push_back(PassEntry{ std::move(pass), {} });
}

ResourceHandle RenderGraph::registerTransientTexture(const TextureResourceDesc& desc) {
    ResourceEntry entry;
    entry.type = ResourceType::Texture;
    entry.isImported = false;
    entry.desc = desc;
    entry.currentLayout = rhi::TextureLayout::Undefined;
    entry.generation = 1;

    uint32_t index = static_cast<uint32_t>(m_resources.size());
    m_resources.push_back(entry);
    return ResourceHandle{ index, entry.generation };
}

ResourceHandle RenderGraph::registerImportedTexture(rhi::TextureHandle handle,
                                                     const TextureResourceDesc& desc) {
    ResourceEntry entry;
    entry.type = ResourceType::Texture;
    entry.isImported = true;
    entry.textureHandle = handle;
    entry.desc = desc;
    entry.currentLayout = rhi::TextureLayout::Undefined; // Annahme: Aufrufer garantiert das
    entry.generation = 1;

    uint32_t index = static_cast<uint32_t>(m_resources.size());
    m_resources.push_back(entry);
    return ResourceHandle{ index, entry.generation };
}

void RenderGraph::recordAccess(uint32_t passIndex, ResourceHandle handle, AccessType access) {
    m_passes[passIndex].accesses.push_back(ResourceAccess{ handle, access });
}

rhi::TextureHandle RenderGraph::resolveTexture(ResourceHandle handle) const {
    return m_resources[handle.index].textureHandle;
}

rhi::TextureLayout RenderGraph::requiredLayoutFor(AccessType access) const {
    switch (access) {
        case AccessType::Write: return rhi::TextureLayout::TransferDst;
        case AccessType::Read:  return rhi::TextureLayout::ShaderReadOnly;
    }
    return rhi::TextureLayout::Undefined;
}

rhi::RHIResult<void> RenderGraph::compile() {
    // Schritt 1: setup() für ALLE Passes aufrufen, bevor irgendetwas
    // tatsächlich angelegt wird – der Graph deklariert erst vollständig,
    // welche Resourcen/Zugriffe existieren, bevor er Entscheidungen trifft
    // (Dependency-Order, künftig: Aliasing). addPass() tut das bewusst
    // NICHT – siehe oben.
    for (uint32_t passIndex = 0; passIndex < m_passes.size(); ++passIndex) {
        RenderGraphBuilder builder(*this, passIndex);
        m_passes[passIndex].pass->setup(builder);
    }

    // Schritt 2: tatsächliche GPU-Texturen für transiente Resourcen anlegen.
    // Keine Topological-Sort nötig – Passes laufen in addPass()-Reihenfolge.
    // Wird relevant, sobald Passes wechselseitig voneinander lesen/schreiben.
    //
    // Transientes Speicher-Aliasing ist hier bewusst NICHT implementiert –
    // lohnt sich erst bei mehreren Transient-Texturen mit klar getrennten
    // Lebenszeitfenstern. Für jetzt legt jede transiente Resource ihre
    // eigene GPU-Textur an.
    for (auto& resource : m_resources) {
        if (resource.isImported) continue; // Lebensdauer nicht unsere Sache

        rhi::TextureDesc desc{
            .width = resource.desc.width,
            .height = resource.desc.height,
            .format = resource.desc.format,
            .usage = resource.desc.usage,
            .debugName = resource.desc.debugName,
        };
        auto textureResult = m_backend.createTexture(desc);
        if (!textureResult) {
            return std::unexpected(textureResult.error());
        }
        resource.textureHandle = *textureResult;
    }
    return {};
}

rhi::RHIResult<void> RenderGraph::execute() {
    auto cmdList = m_backend.createCommandList();
    if (!cmdList) {
        return std::unexpected(rhi::RHIError::Unknown);
    }

    RenderContext ctx(*this);

    for (uint32_t passIndex = 0; passIndex < m_passes.size(); ++passIndex) {
        auto& passEntry = m_passes[passIndex];

        for (auto& access : passEntry.accesses) {
            auto& resource = m_resources[access.handle.index];
            if (resource.type != ResourceType::Texture) continue;

            rhi::TextureLayout required = requiredLayoutFor(access.access);
            if (resource.currentLayout != required) {
                cmdList->transitionTexture(resource.textureHandle, resource.currentLayout, required);
                resource.currentLayout = required;
            }
        }

        passEntry.pass->execute(ctx, *cmdList);
    }

    m_backend.submit(*cmdList);
    return {};
}

// --- RenderGraphBuilder-Methoden ---

ResourceHandle RenderGraphBuilder::createTexture(const TextureResourceDesc& desc) {
    return m_graph.registerTransientTexture(desc);
}

ResourceHandle RenderGraphBuilder::importTexture(rhi::TextureHandle externalHandle,
                                                  const TextureResourceDesc& desc) {
    return m_graph.registerImportedTexture(externalHandle, desc);
}

ResourceHandle RenderGraphBuilder::read(ResourceHandle handle) {
    m_graph.recordAccess(m_passIndex, handle, AccessType::Read);
    return handle;
}

ResourceHandle RenderGraphBuilder::write(ResourceHandle handle) {
    m_graph.recordAccess(m_passIndex, handle, AccessType::Write);
    return handle;
}

// --- RenderContext ---

rhi::TextureHandle RenderContext::resolveTexture(ResourceHandle handle) const {
    return m_graph.resolveTexture(handle);
}

} // namespace axiom::renderer::rendergraph