#pragma once

#include "Layer.h"
#include "axiom/events/EventBus.h"

#include <vector>
#include <memory>

namespace axiom {

    class LayerStack {
    public:
        LayerStack() = default;
        ~LayerStack();

        void PushLayer(std::unique_ptr<Layer> layer, EventBus& eventBus);
        void PushOverlay(std::unique_ptr<Layer> overlay, EventBus& eventBus);

        void PopLayer(Layer* layer);
        void PopOverlay(Layer* overlay);

        auto begin() {
            return m_Layers.begin();
        }
        auto end() {
            return m_Layers.end();
        }

        auto rbegin() {
            return m_Layers.rbegin();
        }
        auto rend() {
            return m_Layers.rend();
        }

    private:
        std::vector<std::unique_ptr<Layer>> m_Layers;
        size_t m_LayerInsertIndex = 0;
    };

}