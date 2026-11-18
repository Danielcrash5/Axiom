/**
 * @file axiom/src/core/Layerstack.cpp
 * @brief Implements the layerstack module.
 */

#include "axiom/core/LayerStack.h"
#include "axiom/events/EventBus.h"

namespace axiom {

    LayerStack::~LayerStack() {
        for (auto &layer : m_layers)
            layer->onDetach();
    }

    void LayerStack::shutdown() {
        for (auto &layer : m_layers)
            layer->onDetach();

        m_layers.clear();
    }

    void LayerStack::pushLayer(std::unique_ptr<Layer> layer,
                               EventBus &eventBus) {
        layer->setEventBus(eventBus);
        layer->onAttach();

        m_layers.emplace(m_layers.begin() + m_layerInsertIndex,
                         std::move(layer));
        m_layerInsertIndex++;
    }

    void LayerStack::pushOverlay(std::unique_ptr<Layer> overlay,
                                 EventBus &eventBus) {
        overlay->setEventBus(eventBus);
        overlay->onAttach();
        m_layers.emplace_back(std::move(overlay));
    }

    void LayerStack::popLayer(Layer *layer) {
        auto it = std::find_if(m_layers.begin(), m_layers.end(),
                               [layer](const std::unique_ptr<Layer> &ptr) {
                                   return ptr.get() == layer;
                               });

        if (it != m_layers.end()) {
            (*it)->onDetach();
            m_layers.erase(it);
            m_layerInsertIndex--;
        }
    }

    void LayerStack::popOverlay(Layer *overlay) {
        auto it = std::find_if(m_layers.begin(), m_layers.end(),
                               [overlay](const std::unique_ptr<Layer> &ptr) {
                                   return ptr.get() == overlay;
                               });

        if (it != m_layers.end()) {
            (*it)->onDetach();
            m_layers.erase(it);
        }
    }

} // namespace axiom