#pragma once

#include "Event.h"

namespace axiom {

    class EventDispatcher {
    public:
        explicit EventDispatcher(Event& event)
            : m_Event(event) {
        }

        template<typename T, typename F>
        bool Dispatch(const F& func) {
            if (m_Event.GetType() == T::GetStaticType()) {
                m_Event.Handled = func(static_cast<T&>(m_Event));
                return true;
            }
            return false;
        }

    private:
        Event& m_Event;
    };

}