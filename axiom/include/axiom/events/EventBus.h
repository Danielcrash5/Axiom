#pragma once

#include <functional>
#include <unordered_map>
#include <vector>
#include <typeindex>
#include <algorithm>
#include <mutex>
#include <queue>
#include <memory>
#include <cstdint>

namespace axiom {

    class EventBus {
    public:
        EventBus() = default;
        ~EventBus() = default;

        EventBus(const EventBus&) = delete;
        EventBus& operator=(const EventBus&) = delete;

        // =========================================================
        // Subscribe
        // =========================================================

        template<typename T>
        using ListenerFn = std::function<bool(T&)>;

        template<typename T>
        uint64_t Subscribe(ListenerFn<T> callback, int priority = 0) {
            std::lock_guard<std::mutex> lock(m_Mutex);

            std::type_index type = std::type_index(typeid(T));

            Listener listener;
            listener.ID = ++m_LastID;
            listener.Priority = priority;

            listener.Callback = [callback](void* event) {
                return callback(*static_cast<T*>(event));
                };

            m_Listeners[type].push_back(listener);
            Sort(type);

            return listener.ID;
        }

        // =========================================================
        // Unsubscribe
        // =========================================================

        template<typename T>
        void Unsubscribe(uint64_t id) {
            std::lock_guard<std::mutex> lock(m_Mutex);

            std::type_index type = std::type_index(typeid(T));

            auto it = m_Listeners.find(type);
            if (it == m_Listeners.end())
                return;

            auto& vec = it->second;

            vec.erase(
                std::remove_if(vec.begin(), vec.end(),
                [id](const Listener& l) {
                    return l.ID == id;
                }),
                vec.end()
            );
        }

        // =========================================================
        // Immediate Publish
        // =========================================================

        template<typename T>
        bool Publish(T& event) {
            std::lock_guard<std::mutex> lock(m_Mutex);
            return PublishInternal(event);
        }

        // =========================================================
        // Enqueue Event
        // =========================================================

        template<typename T>
        void Enqueue(T&& event) {
            std::lock_guard<std::mutex> lock(m_Mutex);

            m_EventQueue.push(
                std::make_unique<QueuedEvent<T>>(std::forward<T>(event))
            );
        }

        // =========================================================
        // Dispatch queued events
        // =========================================================

        void DispatchQueued() {
            std::lock_guard<std::mutex> lock(m_Mutex);

            while (!m_EventQueue.empty()) {
                auto& event = m_EventQueue.front();
                event->Dispatch(*this);
                m_EventQueue.pop();
            }
        }

    private:

        // =========================================================
        // Listener Struct
        // =========================================================

        struct Listener {
            uint64_t ID = 0;
            int Priority = 0;
            std::function<bool(void*)> Callback;
        };

        void Sort(const std::type_index& type) {
            auto& vec = m_Listeners[type];

            std::sort(vec.begin(), vec.end(),
                      [](const Listener& a, const Listener& b) {
                          return a.Priority > b.Priority;
                      });
        }

        // =========================================================
        // Internal Publish
        // =========================================================

        template<typename T>
        bool PublishInternal(T& event) {
            std::type_index type = std::type_index(typeid(T));

            auto it = m_Listeners.find(type);
            if (it == m_Listeners.end())
                return false;

            for (auto& listener : it->second) {
                if (listener.Callback(&event))
                    return true;
            }

            return false;
        }

        // =========================================================
        // Queue Polymorphism
        // =========================================================

        struct IQueuedEvent {
            virtual ~IQueuedEvent() = default;
            virtual void Dispatch(EventBus& bus) = 0;
        };

        template<typename T>
        struct QueuedEvent : IQueuedEvent {
            T EventData;

            QueuedEvent(T&& e)
                : EventData(std::move(e)) {
            }

            void Dispatch(EventBus& bus) override {
                bus.PublishInternal(EventData);
            }
        };

    private:

        std::unordered_map<std::type_index, std::vector<Listener>> m_Listeners;
        std::queue<std::unique_ptr<IQueuedEvent>> m_EventQueue;

        std::mutex m_Mutex;
        uint64_t m_LastID = 0;
    };

} // namespace axiom