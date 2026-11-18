/**
 * @file axiom/include/axiom/renderer/rendergraph/RenderGraph.h
 * @brief Declares the public interface for the render graph.
 */

#pragma once
#include "RenderPass.h"
#include <axiom/renderer/rhi/IRHIBackend.h>
#include <memory>
#include <optional>
#include <unordered_map>
#include <vector>

namespace axiom::renderer::rendergraph {

    /**
     * @brief Builds and executes render passes while tracking resource dependencies.
     * @tparam RenderGraph Template type used by this type.
     */
    class RenderGraph {
      public:
        /**
         * @brief Creates an instance of render graph.
         * @param backend The backend used by this operation.
         */
        explicit RenderGraph(rhi::IRHIBackend &backend) : m_backend(backend) {}
        /**
         * @brief Releases resources owned by RenderGraph.
         */
        ~RenderGraph();

        /**
         * @brief Adds pass.
         * @param pass The pass used by this operation.
         */
        void addPass(std::unique_ptr<RenderPass> pass);
        /**
         * @brief Adds pass first.
         * @param pass The pass used by this operation.
         */
        void addPassFirst(std::unique_ptr<RenderPass> pass);
        /**
         * @brief Clears the stored state.
         */
        void clear();

        /**
         * @brief Performs the pass count operation.
         * @return The result of the operation.
         */
        [[nodiscard]] uint32_t passCount() const {
            return static_cast<uint32_t>(m_passes.size());
        }

        /**
         * @brief Returns the queue descriptors.
         * @return The result of the operation.
         */
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
        /**
         * @brief Compiles the shader source.
         * @param execution The execution used by this operation.
         */
        [[nodiscard]] rhi::RHIResult<void> compile(const RenderExecutionDesc &execution = {});

        // Führt alle Passes in der von compile() bestimmten Reihenfolge aus.
        /**
         * @brief Executes the operation using the supplied context.
         * @param execution The execution used by this operation.
         */
        [[nodiscard]] rhi::RHIResult<void>
        execute(const RenderExecutionDesc &execution = {});

      private:
        friend class RenderGraphBuilder;
        friend class RenderContext;

        /** @brief Stores the data for the resource entry. */
        struct ResourceEntry {
            /** @brief Stores the type. */
            ResourceType type = ResourceType::texture;
            /** @brief Stores the texture handle. */
            rhi::TextureHandle textureHandle; // gültig bei Texture
            /** @brief Stores the current layout. */
            rhi::TextureLayout currentLayout = rhi::TextureLayout::undefined;
            bool isImported =
                /** @brief Stores the false. */
                false; // false = transient, vom Graph selbst erzeugt
            /** @brief Stores the desc. */
            TextureResourceDesc desc;
            /** @brief Stores the generation. */
            uint32_t generation = 0;
            // Wird in execute() nach dem ersten Pass gesetzt, der diese
            // Resource schreibt (siehe RenderContext::isFirstWrite()).
            /** @brief Stores the written this execute. */
            bool writtenThisExecute = false;
        };

        /** @brief Stores the data for the resource access. */
        struct ResourceAccess {
            /** @brief Stores the handle. */
            ResourceHandle handle;
            /** @brief Stores the access. */
            AccessType access;
            // Wenn gesetzt, bestimmt der ZUGRIFF das benoetigte Layout,
            // nicht die TextureUsage der Resource. Noetig, sobald mehrere
            // Passes dieselbe Resource mit verschiedenen Layouts schreiben
            // (z.B. Clear per Transfer, danach Rendern als Attachment).
            /** @brief Stores the layout override. */
            std::optional<rhi::TextureLayout> layoutOverride;
        };

        /** @brief Stores the data for the pass entry. */
        struct PassEntry {
            /** @brief Stores the pass. */
            std::unique_ptr<RenderPass> pass;
            /** @brief Stores the accesses. */
            std::vector<ResourceAccess> accesses;
        };

        // Von RenderGraphBuilder genutzt:
        ResourceHandle
        /**
         * @brief Registers transient texture.
         * @param desc The desc used by this operation.
         * @return The result of the operation.
         */
        registerTransientTexture(const TextureResourceDesc &desc);
        // Dedupliziert nach externem Handle: importieren mehrere Passes
        // dasselbe Image (z.B. das Swapchain-Image), bekommen sie EINEN
        // gemeinsamen ResourceEntry - und damit eine durchgaengige
        // Layout-Historie statt je eines isolierten Entries, der immer bei
        // Undefined anfaengt (ein Uebergang von Undefined darf den Inhalt
        // verwerfen, was das Ergebnis des vorigen Passes zerstoeren wuerde).
        /**
         * @brief Registers imported texture.
         * @param handle The handle used by this operation.
         * @param desc The desc used by this operation.
         * @return The result of the operation.
         */
        ResourceHandle registerImportedTexture(rhi::TextureHandle handle,
                                               const TextureResourceDesc &desc);
        /**
         * @brief Performs the record access operation.
         * @param passIndex The pass index used by this operation.
         * @param handle The handle used by this operation.
         * @param access The access used by this operation.
         * @param layoutOverride The layout override used by this operation.
         */
        void recordAccess(uint32_t passIndex, ResourceHandle handle,
                          AccessType access,
                          std::optional<rhi::TextureLayout> layoutOverride =
                              std::nullopt);

        // Von RenderContext genutzt:
        /**
         * @brief Resolves texture.
         * @param handle The handle used by this operation.
         * @return The result of the operation.
         */
        [[nodiscard]] rhi::TextureHandle
        resolveTexture(ResourceHandle handle) const;
        /**
         * @brief Reports whether first write.
         * @param handle The handle used by this operation.
         * @return The result of the operation.
         */
        [[nodiscard]] bool isFirstWrite(ResourceHandle handle) const;

        // Bestimmt für ein AccessType das benötigte Layout. Fürs
        // Clear-Test-Pass reicht TransferDst; ColorAttachment/ShaderReadOnly
        // kommen mit Pipelines/Sampling in Phase 3.
        /**
         * @brief Performs the required layout for operation.
         * @param resource The resource used by this operation.
         * @param access The access used by this operation.
         * @return The result of the operation.
         */
        [[nodiscard]] rhi::TextureLayout
        requiredLayoutFor(const ResourceEntry &resource, AccessType access) const;

        /**
         * @brief Releases transient resources.
         */
        void releaseTransientResources();

        /** @brief Stores the backend. */
        rhi::IRHIBackend &m_backend;
        /** @brief Stores the passes. */
        std::vector<PassEntry> m_passes;
        /** @brief Stores the resources. */
        std::vector<ResourceEntry> m_resources;
        // Das Present-Target der aktuellen compile()-Execution, EINMAL
        // zentral importiert (ungueltig, wenn die View kein Swapchain-Image
        // hat). Passes greifen ueber RenderGraphBuilder::
        // presentTargetResource() darauf zu, statt selbst zu importieren.
        /** @brief Stores the present resource. */
        ResourceHandle m_presentResource;
        /** @brief Stores the compiled. */
        bool m_compiled = false;
    };

} // namespace axiom::renderer::rendergraph
