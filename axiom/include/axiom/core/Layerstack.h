/**
 * @file axiom/include/axiom/core/Layerstack.h
 * @brief Declares the public interface for the layerstack.
 */

#pragma once

#include "Layer.h"
#include "axiom/events/EventBus.h"

#include <memory>
#include <vector>

namespace axiom {

    /**
     * @brief Maintains the ordered layers and overlays attached to an application.
     * @tparam LayerStack Template type used by this type.
     */
    class LayerStack {
      public:
        /**
         * @brief Creates an instance of layer stack.
         */
        LayerStack() = default;
        /**
         * @brief Releases resources owned by LayerStack.
         */
        ~LayerStack();

        /**
         * @brief Adds layer.
         * @param layer The layer used by this operation.
         * @param eventBus The event bus used by this operation.
         */
        void pushLayer(std::unique_ptr<Layer> layer, EventBus &eventBus);
        /**
         * @brief Adds overlay.
         * @param overlay The overlay used by this operation.
         * @param eventBus The event bus used by this operation.
         */
        void pushOverlay(std::unique_ptr<Layer> overlay, EventBus &eventBus);

        /**
         * @brief Removes layer.
         * @param layer The layer used by this operation.
         */
        void popLayer(Layer *layer);
        /**
         * @brief Removes overlay.
         * @param overlay The overlay used by this operation.
         */
        void popOverlay(Layer *overlay);

        /**
         * @brief Shuts down the subsystem.
         */
        void shutdown();

        /**
         * @brief Begins the operation.
         * @return The result of the operation.
         */
        auto begin() { return m_layers.begin(); }
        /**
         * @brief Ends the operation.
         * @return The result of the operation.
         */
        auto end() { return m_layers.end(); }

        /**
         * @brief Returns a reverse iterator to the last element.
         * @return The result of the operation.
         */
        auto rbegin() { return m_layers.rbegin(); }
        /**
         * @brief Returns a reverse iterator past the first element.
         * @return The result of the operation.
         */
        auto rend() { return m_layers.rend(); }

      private:
        /** @brief Stores the layers. */
        std::vector<std::unique_ptr<Layer>> m_layers;
        /** @brief Stores the layer insert index. */
        size_t m_layerInsertIndex = 0;
    };

} // namespace axiom