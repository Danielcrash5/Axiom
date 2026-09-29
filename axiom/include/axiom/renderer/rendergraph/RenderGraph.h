#pragma once
#include "RenderPass.h"
#include <axiom/renderer/rhi/IRHIBackend.h>
#include <memory>
#include <optional>
#include <unordered_map>
#include <vector>

namespace axiom::renderer::rendergraph {

    class RenderGraph {
      public:
        explicit RenderGraph(rhi::IRHIBackend &backend) : m_backend(backend) {}
        ~RenderGraph();

        void addPass(std::unique_ptr<RenderPass> pass);
        void clear();

        [[nodiscard]] uint32_t passCount() const {
            return static_cast<uint32_t>(m_passes.size());
        }

        [[nodiscard]] std::vector<RenderQueueDescriptor>
        queueDescriptors() const;

        // Löst Dependencies/Layouts auf (welche Barriers wo nötig sind),
        // erzeugt/aktualisiert transiente Resourcen. Wiederverwendet
        // bestehende GPU-Texturen, wenn sich ihre Beschreibung seit dem
        // letzten compile() nicht geändert hat (kein Zerstören/Neuanlegen
        // pro Frame nur weil renderFrame() erneut kompiliert). Keine
        // Szenen-/Sortier-/Batch-Logik (die liegt in
        // RenderQueueSystem/BatchBuilder, Abschnitt 10). `execution` wird an
        // jeden Pass' setup() gereicht (RenderGraphBuilder::currentView()/
        // presentTarget()), damit Passes z.B. View-abhängige Größen wählen
        // können.
        [[nodiscard]] rhi::RHIResult<void> compile(const RenderExecutionDesc &execution = {});

        // Führt alle Passes in der von compile() bestimmten Reihenfolge aus.
        [[nodiscard]] rhi::RHIResult<void>
        execute(const RenderExecutionDesc &execution = {});

      private:
        friend class RenderGraphBuilder;
        friend class RenderContext;

        struct ResourceEntry {
            ResourceType type = ResourceType::Texture;
            rhi::TextureHandle textureHandle; // gültig bei Texture
            rhi::TextureLayout currentLayout = rhi::TextureLayout::Undefined;
            bool isImported =
                false; // false = transient, vom Graph selbst erzeugt
            TextureResourceDesc desc;
            uint32_t generation = 0;
            // Wird in execute() nach dem ersten Pass gesetzt, der diese
            // Resource schreibt (siehe RenderContext::isFirstWrite()).
            bool writtenThisExecute = false;
        };

        struct ResourceAccess {
            ResourceHandle handle;
            AccessType access;
            // Wenn gesetzt, bestimmt der ZUGRIFF das benoetigte Layout,
            // nicht die TextureUsage der Resource. Noetig, sobald mehrere
            // Passes dieselbe Resource mit verschiedenen Layouts schreiben
            // (z.B. Clear per Transfer, danach Rendern als Attachment).
            std::optional<rhi::TextureLayout> layoutOverride;
        };

        struct PassEntry {
            std::unique_ptr<RenderPass> pass;
            std::vector<ResourceAccess> accesses;
        };

        // Von RenderGraphBuilder genutzt:
        ResourceHandle
        registerTransientTexture(const TextureResourceDesc &desc);
        // Dedupliziert nach externem Handle: importieren mehrere Passes
        // dasselbe Image (z.B. das Swapchain-Image), bekommen sie EINEN
        // gemeinsamen ResourceEntry - und damit eine durchgaengige
        // Layout-Historie statt je eines isolierten Entries, der immer bei
        // Undefined anfaengt (ein Uebergang von Undefined darf den Inhalt
        // verwerfen, was das Ergebnis des vorigen Passes zerstoeren wuerde).
        ResourceHandle registerImportedTexture(rhi::TextureHandle handle,
                                               const TextureResourceDesc &desc);
        void recordAccess(uint32_t passIndex, ResourceHandle handle,
                          AccessType access,
                          std::optional<rhi::TextureLayout> layoutOverride =
                              std::nullopt);

        // Von RenderContext genutzt:
        [[nodiscard]] rhi::TextureHandle
        resolveTexture(ResourceHandle handle) const;
        [[nodiscard]] bool isFirstWrite(ResourceHandle handle) const;

        // Bestimmt für ein AccessType das benötigte Layout. Fürs
        // Clear-Test-Pass reicht TransferDst; ColorAttachment/ShaderReadOnly
        // kommen mit Pipelines/Sampling in Phase 3.
        [[nodiscard]] rhi::TextureLayout
        requiredLayoutFor(const ResourceEntry &resource, AccessType access) const;

        void releaseTransientResources();

        rhi::IRHIBackend &m_backend;
        std::vector<PassEntry> m_passes;
        std::vector<ResourceEntry> m_resources;
        // Das Present-Target der aktuellen compile()-Execution, EINMAL
        // zentral importiert (ungueltig, wenn die View kein Swapchain-Image
        // hat). Passes greifen ueber RenderGraphBuilder::
        // presentTargetResource() darauf zu, statt selbst zu importieren.
        ResourceHandle m_presentResource;
        bool m_compiled = false;
    };

} // namespace axiom::renderer::rendergraph
