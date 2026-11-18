/**
 * @file axiom/include/axiom/events/EventBus.h
 * @brief Declares the public interface for the event bus.
 */

#pragma once

#include <algorithm>
#include <cstdint>
#include <functional>
#include <memory>
#include <queue>
#include <typeindex>
#include <unordered_map>
#include <vector>

namespace axiom {

    /**
     * @brief Routes typed events to subscribed listeners.
     * @tparam EventBus Template type used by this type.
     */
    class EventBus {
      public:
        /**
         * @brief Creates an instance of event bus.
         */
        EventBus() = default;
        /**
         * @brief Releases resources owned by EventBus.
         */
        ~EventBus() = default;

        /**
         * @brief Creates an instance of event bus.
         * @param EventBus The event bus used by this operation.
         */
        EventBus(const EventBus &) = delete;
        EventBus &operator=(const EventBus &) = delete;

        // =========================================================
        // Subscribe
        // =========================================================

        template <typename T> using ListenerFn = std::function<bool(T &)>;

        /**
         * @brief Performs the subscribe operation.
         * @tparam T Template type used by this operation.
         * @param callback The callback used by this operation.
         * @param priority The priority used by this operation.
         * @return The result of the operation.
         */
        template <typename T>
        uint64_t subscribe(ListenerFn<T> callback, int priority = 0) {

            std::type_index type = std::type_index(typeid(T));

            Listener listener;
            listener.ID = ++m_lastId;
            listener.Priority = priority;

            listener.Callback = [callback](void *event) {
                return callback(*static_cast<T *>(event));
            };

            m_listeners[type].push_back(listener);
            sort(type);

            return listener.ID;
        }

        // =========================================================
        // Unsubscribe
        // =========================================================

        /**
         * @brief Performs the unsubscribe operation.
         * @tparam T Template type used by this operation.
         * @param id The ID used by this operation.
         */
        template <typename T> void unsubscribe(uint64_t id) {

            std::type_index type = std::type_index(typeid(T));

            auto it = m_listeners.find(type);
            if (it == m_listeners.end())
                return;

            auto &vec = it->second;

            vec.erase(
                std::remove_if(vec.begin(), vec.end(),
                               [id](const Listener &l) { return l.ID == id; }),
                vec.end());
        }

        // =========================================================
        // Immediate Publish
        // =========================================================

        /**
         * @brief Performs the publish operation.
         * @tparam T Template type used by this operation.
         * @param event The event used by this operation.
         * @return The result of the operation.
         */
        template <typename T> bool publish(T &event) {
            return publishInternal(event);
        }

        // =========================================================
        // Enqueue Event
        // =========================================================

        /**
         * @brief Adds work to the queue.
         * @tparam T Template type used by this operation.
         * @param event The event used by this operation.
         */
        template <typename T> void enqueue(T &&event) {

            m_eventQueue.push(
                std::make_unique<QueuedEvent<T>>(std::forward<T>(event)));
        }

        // =========================================================
        // Dispatch queued events
        // =========================================================

        /**
         * @brief Dispatches queued.
         */
        void dispatchQueued() {

            while (!m_eventQueue.empty()) {
                auto &event = m_eventQueue.front();
                event->dispatch(*this);
                m_eventQueue.pop();
            }
        }

      private:
        // =========================================================
        // Listener Struct
        // =========================================================

        /** @brief Stores the data for the listener. */
        struct Listener {
            /** @brief Stores the ID. */
            uint64_t ID = 0;
            /** @brief Stores the priority. */
            int Priority = 0;
            std::function<bool(void *)> Callback;
        };

        /**
         * @brief Sorts the stored items.
         * @param type The type used by this operation.
         */
        void sort(const std::type_index &type) {
            auto &vec = m_listeners[type];

            std::sort(vec.begin(), vec.end(),
                      [](const Listener &a, const Listener &b) {
                          return a.Priority > b.Priority;
                      });
        }

        // =========================================================
        // Internal Publish
        // =========================================================

        /**
         * @brief Publishes internal.
         * @tparam T Template type used by this operation.
         * @param event The event used by this operation.
         * @return The result of the operation.
         */
        template <typename T> bool publishInternal(T &event) {
            std::type_index type = std::type_index(typeid(T));

            auto it = m_listeners.find(type);
            if (it == m_listeners.end())
                return false;

            for (auto &listener : it->second) {
                if (listener.Callback(&event))
                    return true;
            }

            return false;
        }

        // =========================================================
        // Queue Polymorphism
        // =========================================================

        /** @brief Defines the interface for the queued event. */
        struct IQueuedEvent {
            /**
             * @brief Releases resources owned by IQueuedEvent.
             */
            virtual ~IQueuedEvent() = default;
            /**
             * @brief Dispatches the pending event or command.
             * @param bus The bus used by this operation.
             */
            virtual void dispatch(EventBus &bus) = 0;
        };

        template <typename T> struct QueuedEvent : IQueuedEvent {
            T EventData;

            QueuedEvent(T &&e) : EventData(std::move(e)) {}

            void dispatch(EventBus &bus) override {
                bus.publishInternal(EventData);
            }
        };

      private:
        /** @brief Stores the listeners. */
        std::unordered_map<std::type_index, std::vector<Listener>> m_listeners;
        /** @brief Stores the event queue. */
        std::queue<std::unique_ptr<IQueuedEvent>> m_eventQueue;

        /** @brief Stores the last ID. */
        uint64_t m_lastId = 0;
    };

} // namespace axiom