#pragma once

#include "axiom/events/EventBus.h"

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_video.h>

#include <functional>
#include <string>

namespace axiom {

    class Window {
      public:
        struct Props {
            uint32_t width = 1280;
            uint32_t height = 720;
            std::string title = "Engine";
            bool vsync = true;
        };

      public:
        explicit Window(const Props &props, EventBus &eventBus);
        ~Window();

        void PollEvents();
        void SwapBuffers();

        bool ShouldClose() const;

        uint32_t GetWidth() const;
        uint32_t GetHeight() const;

        SDL_Window *GetNativeHandle() const;

        // Sieht jedes rohe SDL_Event VOR der Uebersetzung in EventBus-Events.
        // Rueckgabe true = Event konsumiert (z.B. von ImGui), wird verworfen.
        using NativeEventHook = std::function<bool(const SDL_Event &)>;
        void SetNativeEventHook(NativeEventHook hook) {
            m_NativeEventHook = std::move(hook);
        }

        bool VsyncEnabled() { return m_Vsync; }
        void ToggleVsync();

      private:
        void Init(const Props &props);
        void Shutdown();

      private:
        SDL_Window *m_Window = nullptr;

        EventBus &m_EventBus;
        NativeEventHook m_NativeEventHook;

        bool m_Vsync = true;
        bool m_ShouldClose = false;

        uint32_t m_Width = 0;
        uint32_t m_Height = 0;
    };

} // namespace axiom