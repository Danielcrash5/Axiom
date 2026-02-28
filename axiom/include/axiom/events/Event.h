#pragma once

#include <string>
#include <cstdint>

namespace axiom {

    enum class EventType {
        None = 0,

        // Application
        WindowClose,
        WindowResize,

        // Input
        KeyPressed,
        KeyReleased,
        MouseMoved,
        MouseButtonPressed,
        MouseButtonReleased
    };

    enum EventCategory : uint32_t {
        NoneCategory = 0,
        Application = 1 << 0,
        Window = 1 << 1,
        Input = 1 << 2,
        Keyboard = 1 << 3,
        Mouse = 1 << 4,
        MouseButton = 1 << 5
    };

    class Event {
    public:
        virtual ~Event() = default;

        bool Handled = false;

        virtual EventType GetType() const = 0;
        virtual const char* GetName() const = 0;
        virtual uint32_t GetCategoryFlags() const = 0;

        bool IsInCategory(EventCategory category) const {
            return GetCategoryFlags() & category;
        }

        virtual std::string ToString() const {
            return GetName();
        }
    };

#define AXIOM_EVENT_TYPE(type) \
    static EventType GetStaticType() { return EventType::type; } \
    EventType GetType() const override { return GetStaticType(); } \
    const char* GetName() const override { return #type; }

#define AXIOM_EVENT_CATEGORY(category) \
    uint32_t GetCategoryFlags() const override { return category; }

}