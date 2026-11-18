/**
 * @file axiom/include/axiom/platform/Window.h
 * @brief Declares the public interface for the window.
 */

#pragma once

#include "axiom/events/EventBus.h"

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_video.h>

#include <functional>
#include <string>

namespace axiom {

    /**
     * @brief Owns the native window and forwards its events.
     * @tparam Window Template type used by this type.
     */
    class Window {
      public:
        /** @brief Stores the data for the props. */
        struct Props {
            /** @brief Stores the width. */
            uint32_t width = 1280;
            /** @brief Stores the height. */
            uint32_t height = 720;
            /** @brief Stores the title. */
            std::string title = "Engine";
        };

      public:
        /**
         * @brief Creates an instance of window.
         * @param props The props used by this operation.
         * @param eventBus The event bus used by this operation.
         */
        explicit Window(const Props &props, EventBus &eventBus);
        /**
         * @brief Releases resources owned by Window.
         */
        ~Window();

        /**
         * @brief Performs the poll events operation.
         */
        void pollEvents();

        /**
         * @brief Reports whether close.
         * @return The result of the operation.
         */
        bool shouldClose() const;

        /**
         * @brief Returns the width.
         * @return The result of the operation.
         */
        uint32_t getWidth() const;
        /**
         * @brief Returns the height.
         * @return The result of the operation.
         */
        uint32_t getHeight() const;

        /**
         * @brief Returns the native handle.
         * @return The result of the operation.
         */
        SDL_Window *getNativeHandle() const;

        // Sieht jedes rohe SDL_Event VOR der Uebersetzung in EventBus-Events.
        // Rueckgabe true = Event konsumiert (z.B. von ImGui), wird verworfen.
        /**
         * @brief Provides the native event hook type alias.
         */
        using NativeEventHook = std::function<bool(const SDL_Event &)>;
        /**
         * @brief Sets native event hook.
         * @param hook The hook used by this operation.
         */
        void setNativeEventHook(NativeEventHook hook) {
            m_nativeEventHook = std::move(hook);
        }

      private:
        /**
         * @brief Initializes the subsystem.
         * @param props The props used by this operation.
         */
        void init(const Props &props);
        /**
         * @brief Shuts down the subsystem.
         */
        void shutdown();

      private:
        /** @brief Stores the window. */
        SDL_Window *m_window = nullptr;

        /** @brief Stores the event bus. */
        EventBus &m_eventBus;
        /** @brief Stores the native event hook. */
        NativeEventHook m_nativeEventHook;

        /** @brief Stores the should close. */
        bool m_shouldClose = false;

        /** @brief Stores the width. */
        uint32_t m_width = 0;
        /** @brief Stores the height. */
        uint32_t m_height = 0;
    };

} // namespace axiom