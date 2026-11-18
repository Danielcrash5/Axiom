/**
 * @file axiom/include/axiom/renderer/rendergraph/RenderPass.h
 * @brief Declares the public interface for the render pass.
 */

#pragma once
#include "RenderContext.h"
#include "RenderGraphBuilder.h"
#include <axiom/renderer/RenderQueue.h>
#include <axiom/renderer/rhi/CommandList.h>

namespace axiom::renderer::rendergraph {

    /**
     * @brief Provides the render pass functionality.
     * @tparam RenderPass Template type used by this type.
     */
    class RenderPass {
      public:
        /**
         * @brief Releases resources owned by RenderPass.
         */
        virtual ~RenderPass() = default;
        /**
         * @brief Configures the resources required by this object.
         * @param builder The builder used by this operation.
         */
        virtual void setup(RenderGraphBuilder &builder) = 0;
        /**
         * @brief Executes the operation using the supplied context.
         * @param ctx The ctx used by this operation.
         * @param cmd The cmd used by this operation.
         */
        virtual void execute(RenderContext &ctx, rhi::CommandList &cmd) = 0;

        // Rein informativ (Debug/Profiling), keine Logik daran geknüpft.
        /**
         * @brief Returns the name.
         * @return The result of the operation.
         */
        [[nodiscard]] virtual const char *name() const = 0;

        // Item-basierte Passes koennen hier ihre Queue-Anforderungen melden.
        // Post-FX/Clear/Compute-Passes lassen den Default stehen und bekommen
        // keine RenderItems vorbereitet.
        /**
         * @brief Reports whether this pass uses a render queue.
         * @return The result of the operation.
         */
        [[nodiscard]] virtual bool usesRenderQueue() const { return false; }
        /**
         * @brief Returns the queue descriptor.
         * @return The result of the operation.
         */
        [[nodiscard]] virtual RenderQueueDescriptor queueDescriptor() const {
            return {};
        }
    };

} // namespace axiom::renderer::rendergraph
