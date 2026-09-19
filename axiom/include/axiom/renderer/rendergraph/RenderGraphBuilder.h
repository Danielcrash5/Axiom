#pragma once
#include "RenderGraphTypes.h"
#include "ResourceHandle.h"
#include "RenderContext.h" // fuer RenderExecutionDesc
#include <axiom/renderer/rhi/RHITypes.h>

namespace axiom::renderer::rendergraph {

    class RenderGraph; // fwd

    // Wird nur während RenderPass::setup() gereicht – deklariert Resource-
    // Erzeugung und Lese-/Schreibzugriffe, OHNE selbst Zeichenarbeit zu tun.
    // Bekommt dieselbe RenderExecutionDesc wie RenderContext::execute() -
    // Passes koennen so z.B. Transient-Resourcen an die Groesse der
    // aktuellen View anpassen oder das per-Frame-Swapchain-Image importieren
    // (siehe presentTarget()).
    class RenderGraphBuilder {
      public:
        explicit RenderGraphBuilder(RenderGraph &graph, uint32_t passIndex,
                                    const RenderExecutionDesc &execution = {})
            : m_graph(graph), m_passIndex(passIndex), m_execution(execution) {}

        [[nodiscard]] const View *currentView() const { return m_execution.view; }
        [[nodiscard]] rhi::TextureHandle presentTarget() const {
            return m_execution.presentTarget;
        }

        // Transiente Resource: der Graph erzeugt/verwaltet die GPU-Textur
        // selbst.
        [[nodiscard]] ResourceHandle
        createTexture(const TextureResourceDesc &desc);

        // Importierte Resource: existiert schon extern (z.B. eine Testtextur
        // aus IRHIBackend::createTexture, später: Swapchain-Image aus Phase 8).
        // Der Graph übernimmt NICHT die Lebensdauer, nur das State-Tracking.
        [[nodiscard]] ResourceHandle
        importTexture(rhi::TextureHandle externalHandle,
                      const TextureResourceDesc &desc);

        // Deklariert Zugriff für den aktuellen Pass – wird für Dependency-Order
        // und automatische Barrier-Einfügung in RenderGraph::compile() genutzt.
        ResourceHandle read(ResourceHandle handle);
        ResourceHandle write(ResourceHandle handle);

      private:
        RenderGraph &m_graph;
        uint32_t m_passIndex;
        RenderExecutionDesc m_execution;
    };

} // namespace axiom::renderer::rendergraph