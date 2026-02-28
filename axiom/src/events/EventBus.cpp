#include "axiom/events/EventBus.h"

namespace axiom {

    ListenerHandle EventBus::Subscribe(
        EventType type,
        ListenerFn callback,
        EventPriority priority) {
        std::lock_guard lock(m_ListenerMutex);

        Listener listener;
        listener.ID = ++m_LastID;
        listener.Priority = priority;
        listener.Callback = callback;

        m_Listeners[type].push_back(listener);
        Sort(type);

        return { listener.ID, type };
    }

    void EventBus::Unsubscribe(const ListenerHandle& handle) {
        std::lock_guard lock(m_ListenerMutex);

        auto& vec = m_Listeners[handle.Type];

        vec.erase(
            std::remove_if(vec.begin(), vec.end(),
            [&](const Listener& l) {
                return l.ID == handle.ID;
            }),
            vec.end());
    }

    void EventBus::Publish(Event& event) {
        std::lock_guard lock(m_ListenerMutex);

        auto it = m_Listeners.find(event.GetType());
        if (it == m_Listeners.end())
            return;

        for (auto& listener : it->second) {
            if (event.Handled)
                break;

            event.Handled = listener.Callback(event);
        }
    }

    void EventBus::QueueEvent(std::unique_ptr<Event> event) {
        std::lock_guard lock(m_QueueMutex);
        m_Queue.push(std::move(event));
    }

    void EventBus::ProcessQueue() {
        std::queue<std::unique_ptr<Event>> localQueue;

        {
            std::lock_guard lock(m_QueueMutex);
            std::swap(localQueue, m_Queue);
        }

        while (!localQueue.empty()) {
            Publish(*localQueue.front());
            localQueue.pop();
        }
    }

    void EventBus::Sort(EventType type) {
        auto& vec = m_Listeners[type];

        std::sort(vec.begin(), vec.end(),
                  [](const Listener& a, const Listener& b) {
                      return a.Priority < b.Priority;
                  });
    }

}