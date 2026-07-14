#pragma once

#include <string>

#include "axiom/events/EventBus.h"

namespace axiom {

    class Event;

    class Layer {
      public:
        Layer(std::string name = "Layer") : m_Name(std::move(name)) {}

        virtual ~Layer() = default;

        virtual void OnAttach() {}
        virtual void OnDetach() {}

        virtual void OnPreUpdate(double dt) {}
        virtual void OnPostUpdate(double dt) {}
        virtual void OnFixedUpdate(double dt) {}
        virtual void OnUpdate(double dt) {}

        virtual void OnRender(double alpha) {}
        virtual void OnImGuiRender() {}

        void setEventBus(EventBus &eventBus) { m_EventBus = &eventBus; }

        const std::string &GetName() const { return m_Name; }

      protected:
        std::string m_Name;
        EventBus *m_EventBus = nullptr;
    };

} // namespace axiom