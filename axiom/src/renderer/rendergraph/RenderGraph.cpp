#include <axiom/renderer/rendergraph/RenderGraph.h>

namespace axiom::renderer::rendergraph {

    RenderGraph::~RenderGraph() { clear(); }

    void RenderGraph::addPass(std::unique_ptr<RenderPass> pass) {
        // Bewusst NUR speichern – setup() wird erst in compile() aufgerufen,
        // damit der Graph erst alle Passes vollständig kennt, bevor er
        // irgendetwas anlegt oder Entscheidungen trifft.
        if (!pass) {
            return;
        }
        m_passes.push_back(PassEntry{std::move(pass), {}});
        m_compiled = false;
    }

    void RenderGraph::clear() {
        releaseTransientResources();
        m_resources.clear();
        m_passes.clear();
        m_compiled = false;
    }

    std::vector<RenderQueueDescriptor> RenderGraph::queueDescriptors() const {
        std::vector<RenderQueueDescriptor> descriptors;
        descriptors.reserve(m_passes.size());

        for (const PassEntry& passEntry : m_passes) {
            descriptors.push_back(passEntry.pass->usesRenderQueue()
                                      ? passEntry.pass->queueDescriptor()
                                      : RenderQueueDescriptor{.acceptedLayers = 0});
        }

        return descriptors;
    }

    ResourceHandle
    RenderGraph::registerTransientTexture(const TextureResourceDesc &desc) {
        ResourceEntry entry;
        entry.type = ResourceType::Texture;
        entry.isImported = false;
        entry.desc = desc;
        entry.currentLayout = rhi::TextureLayout::Undefined;
        entry.generation = 1;

        uint32_t index = static_cast<uint32_t>(m_resources.size());
        m_resources.push_back(entry);
        return ResourceHandle{index, entry.generation};
    }

    ResourceHandle
    RenderGraph::registerImportedTexture(rhi::TextureHandle handle,
                                         const TextureResourceDesc &desc) {
        ResourceEntry entry;
        entry.type = ResourceType::Texture;
        entry.isImported = true;
        entry.textureHandle = handle;
        entry.desc = desc;
        entry.currentLayout =
            rhi::TextureLayout::Undefined; // Annahme: Aufrufer garantiert das
        entry.generation = 1;

        uint32_t index = static_cast<uint32_t>(m_resources.size());
        m_resources.push_back(entry);
        return ResourceHandle{index, entry.generation};
    }

    void RenderGraph::recordAccess(uint32_t passIndex, ResourceHandle handle,
                                   AccessType access) {
        m_passes[passIndex].accesses.push_back(ResourceAccess{handle, access});
    }

    rhi::TextureHandle
    RenderGraph::resolveTexture(ResourceHandle handle) const {
        return m_resources[handle.index].textureHandle;
    }

    rhi::TextureLayout RenderGraph::requiredLayoutFor(
        const ResourceEntry &resource, AccessType access) const {
        switch (access) {
        case AccessType::Write:
            if (resource.desc.usage & rhi::TextureUsage::RenderTarget) {
                return rhi::TextureLayout::ColorAttachment;
            }
            return rhi::TextureLayout::TransferDst;
        case AccessType::Read:
            return rhi::TextureLayout::ShaderReadOnly;
        }
        return rhi::TextureLayout::Undefined;
    }

    rhi::RHIResult<void> RenderGraph::compile(const RenderExecutionDesc &execution) {
        // Alte Resource-Beschreibungen VOR dem Ueberschreiben sichern, um
        // unveraenderte transiente Resourcen unten wiederzuverwenden, statt
        // sie bei jedem compile()-Aufruf (z.B. einmal pro View pro Frame in
        // Renderer::renderFrame()) bedingungslos zu zerstoeren und neu
        // anzulegen. Import-Resourcen kosten dabei ohnehin nichts (siehe
        // Schritt 2), nur transiente GPU-Texturen sind teuer.
        std::vector<ResourceEntry> previousResources = std::move(m_resources);
        m_resources.clear();
        for (auto &passEntry : m_passes) {
            passEntry.accesses.clear();
        }
        m_compiled = false;

        // Schritt 1: setup() für ALLE Passes aufrufen, bevor irgendetwas
        // tatsächlich angelegt wird – der Graph deklariert erst vollständig,
        // welche Resourcen/Zugriffe existieren, bevor er Entscheidungen trifft
        // (Dependency-Order, künftig: Aliasing). addPass() tut das bewusst
        // NICHT – siehe oben. `execution` wird durchgereicht, damit Passes
        // waehrend setup() z.B. die aktuelle View oder ein per-Frame
        // Present-Target (Swapchain-Image) kennen.
        for (uint32_t passIndex = 0; passIndex < m_passes.size(); ++passIndex) {
            RenderGraphBuilder builder(*this, passIndex, execution);
            m_passes[passIndex].pass->setup(builder);
        }

        // Schritt 2: tatsächliche GPU-Texturen für transiente Resourcen
        // anlegen - ABER nur, wenn sich ihre Beschreibung gegenüber dem
        // vorigen compile() geändert hat (Groesse/Format/Usage). Die
        // Reihenfolge ist stabil, weil Passes ihre setup()-Reihenfolge nicht
        // ändern, daher entspricht Index i in previousResources derselben
        // deklarierten Resource wie Index i in m_resources.
        //
        // Keine Topological-Sort nötig – Passes laufen in addPass()-
        // Reihenfolge. Transientes Speicher-Aliasing ist weiterhin bewusst
        // NICHT implementiert – jede transiente Resource hat ihre eigene
        // GPU-Textur, nur eben nicht mehr pro compile()-Aufruf neu.
        for (size_t i = 0; i < m_resources.size(); ++i) {
            auto &resource = m_resources[i];
            if (resource.isImported)
                continue; // Lebensdauer nicht unsere Sache

            bool canReuse = i < previousResources.size() &&
                            !previousResources[i].isImported &&
                            previousResources[i].textureHandle.valid() &&
                            previousResources[i].desc.width == resource.desc.width &&
                            previousResources[i].desc.height == resource.desc.height &&
                            previousResources[i].desc.format == resource.desc.format &&
                            previousResources[i].desc.usage == resource.desc.usage;

            if (canReuse) {
                resource.textureHandle = previousResources[i].textureHandle;
                resource.currentLayout = previousResources[i].currentLayout;
                previousResources[i].textureHandle = {}; // als "übernommen" markieren
                continue;
            }

            rhi::TextureDesc desc{
                .width = resource.desc.width,
                .height = resource.desc.height,
                .format = resource.desc.format,
                .usage = resource.desc.usage,
                .debugName = resource.desc.debugName,
            };
            auto textureResult = m_backend.createTexture(desc);
            if (!textureResult) {
                // Alles, was wir aus previousResources noch nicht als
                // "übernommen" markiert haben, ist jetzt verwaist - sauber
                // aufräumen, bevor wir mit Fehler zurückkehren.
                for (auto &old : previousResources) {
                    if (!old.isImported && old.textureHandle.valid()) {
                        m_backend.destroyTexture(old.textureHandle);
                    }
                }
                return std::unexpected(textureResult.error());
            }
            resource.textureHandle = *textureResult;
        }

        // Schritt 3: alles aus dem letzten compile(), das NICHT übernommen
        // wurde (textureHandle wurde oben auf {} zurückgesetzt, wenn
        // übernommen), jetzt zerstören. Absichtlich NACH Schritt 2, damit wir
        // uns nicht selbst die Textur wegreißen, die wir gerade
        // wiederverwenden wollen.
        for (auto &old : previousResources) {
            if (!old.isImported && old.textureHandle.valid()) {
                m_backend.destroyTexture(old.textureHandle);
            }
        }

        m_compiled = true;
        return {};
    }

    rhi::RHIResult<void> RenderGraph::execute(const RenderExecutionDesc &execution) {
        if (!m_compiled) {
            return std::unexpected(rhi::RHIError::InvalidDescriptor);
        }

        auto cmdList = m_backend.createCommandList();
        if (!cmdList) {
            return std::unexpected(rhi::RHIError::Unknown);
        }

        RenderContext ctx(*this, execution);

        for (uint32_t passIndex = 0; passIndex < m_passes.size(); ++passIndex) {
            auto &passEntry = m_passes[passIndex];
            ctx.setCurrentPassIndex(passIndex);

            for (auto &access : passEntry.accesses) {
                auto &resource = m_resources[access.handle.index];
                if (resource.type != ResourceType::Texture)
                    continue;

                rhi::TextureLayout required = requiredLayoutFor(resource, access.access);
                if (resource.currentLayout != required) {
                    cmdList->transitionTexture(resource.textureHandle,
                                               resource.currentLayout,
                                               required);
                    resource.currentLayout = required;
                }
            }

            passEntry.pass->execute(ctx, *cmdList);
        }

        // Wenn diese View auf eine Swapchain zielt: das Present-Target muss
        // vor vkQueuePresentKHR im PRESENT_SRC_KHR-Layout sein - das passiert
        // sonst nirgends automatisch (Passes kennen nur Read/Write, nicht
        // "wird gleich praesentiert"). Renderer::renderFrame() ruft
        // present() direkt nach execute() auf, das Layout muss also HIER,
        // im letzten CommandList vor dem Submit, gesetzt werden.
        if (execution.presentTarget.valid()) {
            for (auto &resource : m_resources) {
                if (resource.type == ResourceType::Texture &&
                    resource.textureHandle == execution.presentTarget) {
                    if (resource.currentLayout != rhi::TextureLayout::Present) {
                        cmdList->transitionTexture(resource.textureHandle,
                                                   resource.currentLayout,
                                                   rhi::TextureLayout::Present);
                        resource.currentLayout = rhi::TextureLayout::Present;
                    }
                    break;
                }
            }
        }

        m_backend.submit(*cmdList);
        return {};
    }

    void RenderGraph::releaseTransientResources() {
        for (auto &resource : m_resources) {
            if (!resource.isImported && resource.textureHandle.valid()) {
                m_backend.destroyTexture(resource.textureHandle);
                resource.textureHandle = {};
                resource.currentLayout = rhi::TextureLayout::Undefined;
            }
        }
    }

    // --- RenderGraphBuilder-Methoden ---

    ResourceHandle
    RenderGraphBuilder::createTexture(const TextureResourceDesc &desc) {
        return m_graph.registerTransientTexture(desc);
    }

    ResourceHandle
    RenderGraphBuilder::importTexture(rhi::TextureHandle externalHandle,
                                      const TextureResourceDesc &desc) {
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

    rhi::TextureHandle
    RenderContext::resolveTexture(ResourceHandle handle) const {
        return m_graph.resolveTexture(handle);
    }

    std::span<const RenderItem> RenderContext::items() const {
        return itemsForPass(m_currentPassIndex);
    }

    std::span<const RenderItem>
    RenderContext::itemsForPass(uint32_t passIndex) const {
        if (passIndex >= m_execution.queuedItemsByPass.size()) {
            return {};
        }
        return m_execution.queuedItemsByPass[passIndex];
    }

} // namespace axiom::renderer::rendergraph
