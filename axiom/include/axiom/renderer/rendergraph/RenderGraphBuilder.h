/**
 * @file axiom/include/axiom/renderer/rendergraph/RenderGraphBuilder.h
 * @brief Declares the public interface for the render graph builder.
 */

#pragma once
#include "RenderGraphTypes.h"
#include "ResourceHandle.h"
#include "RenderContext.h" // fuer RenderExecutionDesc
#include <axiom/renderer/rhi/Commandlist.h> // TextureLayout
#include <axiom/renderer/rhi/RHITypes.h>

namespace axiom::renderer::rendergraph {

    class RenderGraph; // fwd

    // Wird nur während RenderPass::setup() gereicht – deklariert Resource-
    // Erzeugung und Lese-/Schreibzugriffe, OHNE selbst Zeichenarbeit zu tun.
    // Bekommt dieselbe RenderExecutionDesc wie RenderContext::execute() -
    // Passes koennen so z.B. Transient-Resourcen an die Groesse der
    // aktuellen View anpassen oder das per-Frame-Swapchain-Image importieren
    // (siehe presentTarget()).
    /**
     * @brief Provides the render graph builder functionality.
     * @tparam RenderGraphBuilder Template type used by this type.
     */
    class RenderGraphBuilder {
      public:
        /**
         * @brief Creates an instance of render graph builder.
         * @param graph The graph used by this operation.
         * @param passIndex The pass index used by this operation.
         * @param execution The execution used by this operation.
         */
        explicit RenderGraphBuilder(RenderGraph &graph, uint32_t passIndex,
                                    const RenderExecutionDesc &execution = {})
            : m_graph(graph), m_passIndex(passIndex), m_execution(execution) {}

        /**
         * @brief Returns the current view.
         * @return The result of the operation.
         */
        [[nodiscard]] const View *currentView() const { return m_execution.view; }
        /**
         * @brief Performs the present target operation.
         * @return The result of the operation.
         */
        [[nodiscard]] rhi::TextureHandle presentTarget() const {
            return m_execution.presentTarget;
        }

        // Das Present-Target dieses Frames als Graph-Resource - vom Graph
        // EINMAL pro compile() importiert, alle Passes teilen sich denselben
        // ResourceHandle (und damit dieselbe Layout-Historie). Ungueltig
        // (valid() == false), wenn die View kein Swapchain-Image hat.
        /**
         * @brief Performs the present target resource operation.
         * @return The result of the operation.
         */
        [[nodiscard]] ResourceHandle presentTargetResource() const;

        // Transiente Resource: der Graph erzeugt/verwaltet die GPU-Textur
        // selbst.
        /**
         * @brief Creates texture.
         * @param desc The desc used by this operation.
         * @return The result of the operation.
         */
        [[nodiscard]] ResourceHandle
        createTexture(const TextureResourceDesc &desc);

        // Importierte Resource: existiert schon extern (z.B. eine Testtextur
        // aus IRHIBackend::createTexture, später: Swapchain-Image aus Phase 8).
        // Der Graph übernimmt NICHT die Lebensdauer, nur das State-Tracking.
        /**
         * @brief Performs the import texture operation.
         * @param externalHandle The external handle used by this operation.
         * @param desc The desc used by this operation.
         * @return The result of the operation.
         */
        [[nodiscard]] ResourceHandle
        importTexture(rhi::TextureHandle externalHandle,
                      const TextureResourceDesc &desc);

        // Deklariert Zugriff für den aktuellen Pass – wird für Dependency-Order
        // und automatische Barrier-Einfügung in RenderGraph::compile() genutzt.
        /**
         * @brief Performs the read operation.
         * @param handle The handle used by this operation.
         * @return The result of the operation.
         */
        ResourceHandle read(ResourceHandle handle);
        /**
         * @brief Writes data to the destination.
         * @param handle The handle used by this operation.
         * @return The result of the operation.
         */
        ResourceHandle write(ResourceHandle handle);
        // Wie write(), aber der Zugriff legt das Layout fest (statt der
        // TextureUsage der Resource) - z.B. TransferDst fuer einen Clear,
        // ColorAttachment fuer Rendern, beides auf derselben Resource.
        /**
         * @brief Writes data to the destination.
         * @param handle The handle used by this operation.
         * @param layout The layout used by this operation.
         * @return The result of the operation.
         */
        ResourceHandle write(ResourceHandle handle, rhi::TextureLayout layout);

      private:
        /** @brief Stores the graph. */
        RenderGraph &m_graph;
        /** @brief Stores the pass index. */
        uint32_t m_passIndex;
        /** @brief Stores the execution. */
        RenderExecutionDesc m_execution;
    };

} // namespace axiom::renderer::rendergraph