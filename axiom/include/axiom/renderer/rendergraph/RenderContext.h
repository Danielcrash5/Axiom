#pragma once
#include "ResourceHandle.h"
#include <axiom/renderer/RenderItem.h>
#include <axiom/renderer/View.h>
#include <axiom/renderer/rhi/RHITypes.h>
#include <span>
#include <vector>

namespace axiom::renderer::rendergraph {

    class RenderGraph;

    struct RenderExecutionDesc {
        const View *view = nullptr;
        std::span<const std::vector<RenderItem>> queuedItemsByPass;
        // Fuer Views mit RenderTargetKind::Swapchain: das fuer DIESEN Frame
        // per acquireNextImage() geholte Swapchain-Image. Passes importieren
        // es ueber RenderGraphBuilder::presentTarget() waehrend setup() (setup()
        // hat jetzt Zugriff auf dieselbe RenderExecutionDesc wie execute()).
        rhi::TextureHandle presentTarget;
    };

    // Wird an RenderPass::execute() gereicht - Passes loesen ihre in setup()
    // deklarierten ResourceHandles hier auf echte rhi::TextureHandle auf und
    // koennen optional die aktuelle View sowie ihre vorbereitete RenderQueue
    // lesen. Der Graph bleibt Scheduling-Infrastruktur; Queue-Aufbau passiert
    // davor im Renderer.
    class RenderContext {
      public:
        explicit RenderContext(RenderGraph &graph,
                               const RenderExecutionDesc &execution = {})
            : m_graph(graph), m_execution(execution) {}

        [[nodiscard]] rhi::TextureHandle
        resolveTexture(ResourceHandle handle) const;

        [[nodiscard]] const View *currentView() const {
            return m_execution.view;
        }

        [[nodiscard]] std::span<const RenderItem> items() const;
        [[nodiscard]] std::span<const RenderItem> itemsForPass(uint32_t passIndex) const;

      private:
        friend class RenderGraph;

        void setCurrentPassIndex(uint32_t passIndex) {
            m_currentPassIndex = passIndex;
        }

        RenderGraph &m_graph;
        RenderExecutionDesc m_execution;
        uint32_t m_currentPassIndex = 0;
    };

} // namespace axiom::renderer::rendergraph
