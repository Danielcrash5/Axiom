#pragma once

#include "Event.h"

#include <unordered_map>
#include <vector>
#include <functional>
#include <queue>
#include <memory>
#include <mutex>
#include <algorithm>

namespace axiom {

    enum class EventPriority {
        High = 0,
        Normal = 1,
        Low = 2
    };

    struct ListenerHandle {
        uint64_t ID;
        EventType Type;
    };

    class EventBus {
    public:
        using ListenerFn = std::function<bool(Event&)>;

        ListenerHandle Subscribe(
            EventType type,
            ListenerFn callback,
            EventPriority priority = EventPriority::Normal);

        void Unsubscribe(const ListenerHandle& handle);

        void Publish(Event& event);

        void QueueEvent(std::unique_ptr<Event> event);

        void ProcessQueue();

    private:
        struct Listener {
            uint64_t ID;
            EventPriority Priority;
            ListenerFn Callback;
        };

        void Sort(EventType type);

    private:
        std::unordered_map<EventType, std::vector<Listener>> m_Listeners;

        std::queue<std::unique_ptr<Event>> m_Queue;

        std::mutex m_ListenerMutex;
        std::mutex m_QueueMutex;

        uint64_t m_LastID = 0;
    };

}