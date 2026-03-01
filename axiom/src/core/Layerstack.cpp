#include "axiom/core/LayerStack.h"
#include "axiom/events/EventBus.h"

namespace axiom {

    LayerStack::~LayerStack() {
        for (auto& layer : m_Layers)
            layer->OnDetach();
    }

    void LayerStack::PushLayer(std::unique_ptr<Layer> layer, EventBus& eventBus) {
		layer->setEventBus(eventBus);
        layer->OnAttach();

        m_Layers.emplace(m_Layers.begin() + m_LayerInsertIndex, std::move(layer));
        m_LayerInsertIndex++;
    }

    void LayerStack::PushOverlay(std::unique_ptr<Layer> overlay, EventBus& eventBus) {
        overlay->setEventBus(eventBus);
        overlay->OnAttach();
        m_Layers.emplace_back(std::move(overlay));
    }

    void LayerStack::PopLayer(Layer* layer) {
        auto it = std::find_if(m_Layers.begin(), m_Layers.end(),
                               [layer](const std::unique_ptr<Layer>& ptr) {
                                   return ptr.get() == layer;
                               });

        if (it != m_Layers.end()) {
            (*it)->OnDetach();
            m_Layers.erase(it);
            m_LayerInsertIndex--;
        }
    }

    void LayerStack::PopOverlay(Layer* overlay) {
        auto it = std::find_if(m_Layers.begin(), m_Layers.end(),
                               [overlay](const std::unique_ptr<Layer>& ptr) {
                                   return ptr.get() == overlay;
                               });

        if (it != m_Layers.end()) {
            (*it)->OnDetach();
            m_Layers.erase(it);
        }
    }

}