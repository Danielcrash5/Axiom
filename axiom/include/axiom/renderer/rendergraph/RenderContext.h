/**
 * @file axiom/include/axiom/renderer/rendergraph/RenderContext.h
 * @brief Declares the public interface for the render context.
 */

#pragma once
#include "ResourceHandle.h"
#include <axiom/renderer/RenderItem.h>
#include <axiom/renderer/View.h>
#include <axiom/renderer/rhi/RHITypes.h>
#include <span>
#include <vector>

namespace axiom::renderer::rendergraph {

    class RenderGraph;

    /** @brief Stores the data for the render execution desc. */
    struct RenderExecutionDesc {
        /** @brief Stores the view. */
        const View *view = nullptr;
        /** @brief Stores the queued items by pass. */
        std::span<const std::vector<RenderItem>> queuedItemsByPass;
        // Fuer Views mit RenderTargetKind::Swapchain: das fuer DIESEN Frame
        // per acquireNextImage() geholte Swapchain-Image. Passes importieren
        // es ueber RenderGraphBuilder::presentTarget() waehrend setup() (setup()
        // hat jetzt Zugriff auf dieselbe RenderExecutionDesc wie execute()).
        /** @brief Stores the present target. */
        rhi::TextureHandle presentTarget;
    };

    // Wird an RenderPass::execute() gereicht - Passes loesen ihre in setup()
    // deklarierten ResourceHandles hier auf echte rhi::TextureHandle auf und
    // koennen optional die aktuelle View sowie ihre vorbereitete RenderQueue
    // lesen. Der Graph bleibt Scheduling-Infrastruktur; Queue-Aufbau passiert
    // davor im Renderer.
    /**
     * @brief Provides the render context functionality.
     * @tparam RenderContext Template type used by this type.
     */
    class RenderContext {
      public:
        /**
         * @brief Creates an instance of render context.
         * @param graph The graph used by this operation.
         * @param execution The execution used by this operation.
         */
        explicit RenderContext(RenderGraph &graph,
                               const RenderExecutionDesc &execution = {})
            : m_graph(graph), m_execution(execution) {}

        /**
         * @brief Resolves texture.
         * @param handle The handle used by this operation.
         * @return The result of the operation.
         */
        [[nodiscard]] rhi::TextureHandle
        resolveTexture(ResourceHandle handle) const;

        /**
         * @brief Returns the current view.
         * @return The result of the operation.
         */
        [[nodiscard]] const View *currentView() const {
            return m_execution.view;
        }

        // true, solange in diesem execute() noch KEIN vorheriger Pass diese
        // Resource geschrieben hat. Ein Pass, der auf ein geteiltes Ziel
        // schreibt (Swapchain-Image), cleart nur, wenn er der erste
        // Schreiber ist - sonst laedt er (loadOp = LOAD) und ueberzeichnet.
        // So haengt das Clear-Verhalten nicht an einer Pass-Reihenfolge-
        // Konvention, sondern an dem, was der Graph tatsaechlich weiss.
        /**
         * @brief Reports whether first write.
         * @param handle The handle used by this operation.
         * @return The result of the operation.
         */
        [[nodiscard]] bool isFirstWrite(ResourceHandle handle) const;

        /**
         * @brief Performs the items operation.
         * @return The result of the operation.
         */
        [[nodiscard]] std::span<const RenderItem> items() const;
        /**
         * @brief Performs the items for pass operation.
         * @param passIndex The pass index used by this operation.
         * @return The result of the operation.
         */
        [[nodiscard]] std::span<const RenderItem> itemsForPass(uint32_t passIndex) const;

      private:
        friend class RenderGraph;

        /**
         * @brief Sets current pass index.
         * @param passIndex The pass index used by this operation.
         */
        void setCurrentPassIndex(uint32_t passIndex) {
            m_currentPassIndex = passIndex;
        }

        /** @brief Stores the graph. */
        RenderGraph &m_graph;
        /** @brief Stores the execution. */
        RenderExecutionDesc m_execution;
        /** @brief Stores the current pass index. */
        uint32_t m_currentPassIndex = 0;
    };

} // namespace axiom::renderer::rendergraph
