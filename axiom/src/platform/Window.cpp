#include "axiom/platform/Window.h"

#include "axiom/core/Logger.h"
#include "axiom/events/EventBus.h"
#include "axiom/events/Events.h"
#include "axiom/input/Input.h"
#include <imgui_impl_sdl3.h>

#include <SDL3/SDL.h>

#include <stdexcept>
#include <string>

namespace axiom {

    static bool s_SDLInitialized = false;

    Window::Window(const Props &props, EventBus &eventBus)
        : m_EventBus(eventBus) {
        Init(props);
    }

    Window::~Window() { Shutdown(); }

    void Window::Init(const Props &props) {
        if (!s_SDLInitialized) {
            if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD |
                          SDL_INIT_JOYSTICK | SDL_INIT_HAPTIC |
                          SDL_INIT_SENSOR | SDL_INIT_AUDIO))
                throw std::runtime_error(
                    std::string("Failed to initialize SDL3: ") +
                    SDL_GetError());

            s_SDLInitialized = true;
        }

        SDL_WindowFlags flags = SDL_WINDOW_RESIZABLE;

        flags |= SDL_WINDOW_VULKAN;

        m_Window =
            SDL_CreateWindow(props.title.c_str(), static_cast<int>(props.width),
                             static_cast<int>(props.height), flags);
        if (!m_Window)
            throw std::runtime_error(
                std::string("Failed to create SDL3 window: ") + SDL_GetError());

        m_Vsync = props.vsync;

        m_Width = props.width;
        m_Height = props.height;
    }

    void Window::Shutdown() {
        Input::Shutdown();

        if (m_Window) {
            SDL_DestroyWindow(m_Window);
            m_Window = nullptr;
        }

        if (s_SDLInitialized) {
            SDL_Quit();
            s_SDLInitialized = false;
        }
    }

    void Window::PollEvents() {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            // ImGui_ImplSDL3_ProcessEvent(&event);
            switch (event.type) {
            case SDL_EVENT_QUIT: {
                m_ShouldClose = true;
                WindowCloseEvent closeEvent;
                m_EventBus.Publish(closeEvent);
                break;
            }
            case SDL_EVENT_WINDOW_CLOSE_REQUESTED: {
                if (event.window.windowID != SDL_GetWindowID(m_Window))
                    break;

                m_ShouldClose = true;
                WindowCloseEvent closeEvent;
                m_EventBus.Publish(closeEvent);
                break;
            }
            case SDL_EVENT_WINDOW_RESIZED: {
                if (event.window.windowID != SDL_GetWindowID(m_Window))
                    break;

                m_Width = static_cast<uint32_t>(event.window.data1);
                m_Height = static_cast<uint32_t>(event.window.data2);
                WindowResizeEvent resizeEvent{m_Width, m_Height};
                m_EventBus.Publish(resizeEvent);
                break;
            }
            case SDL_EVENT_KEY_DOWN:
            case SDL_EVENT_KEY_UP: {
                KeyEvent keyEvent{static_cast<int>(event.key.key),
                                  static_cast<int>(event.key.scancode),
                                  event.type == SDL_EVENT_KEY_DOWN ? 1 : 0,
                                  static_cast<int>(event.key.mod),
                                  event.key.repeat ? 1 : 0};
                m_EventBus.Publish(keyEvent);
                break;
            }
            case SDL_EVENT_MOUSE_BUTTON_DOWN:
            case SDL_EVENT_MOUSE_BUTTON_UP: {
                MouseButtonEvent mouseButtonEvent{
                    static_cast<int>(event.button.button),
                    event.type == SDL_EVENT_MOUSE_BUTTON_DOWN ? 1 : 0, 0};
                m_EventBus.Publish(mouseButtonEvent);
                break;
            }
            case SDL_EVENT_MOUSE_MOTION: {
                MouseMoveEvent mouseMoveEvent{
                    static_cast<double>(event.motion.x),
                    static_cast<double>(event.motion.y)};
                m_EventBus.Publish(mouseMoveEvent);
                break;
            }
            case SDL_EVENT_MOUSE_WHEEL: {
                Input::OnMouseScroll(event.wheel.x, event.wheel.y);
                MouseScrollEvent scrollEvent{
                    static_cast<double>(event.wheel.x),
                    static_cast<double>(event.wheel.y)};
                m_EventBus.Publish(scrollEvent);
                break;
            }
            case SDL_EVENT_TEXT_INPUT: {
                const char *text = event.text.text;
                if (text) {
                    while (*text) {
                        CharEvent charEvent{static_cast<unsigned int>(*text)};
                        m_EventBus.Publish(charEvent);
                        ++text;
                    }
                }
                break;
            }
            case SDL_EVENT_DROP_FILE: {
                const char *path = event.drop.data;
                FileDropEvent dropEvent{path ? 1 : 0, &path};
                m_EventBus.Publish(dropEvent);
                break;
            }
            case SDL_EVENT_GAMEPAD_ADDED:
            case SDL_EVENT_GAMEPAD_REMOVED:
                Input::OnGamepadChanged();
                break;
            case SDL_EVENT_JOYSTICK_ADDED:
            case SDL_EVENT_JOYSTICK_REMOVED:
                Input::OnJoystickChanged();
                break;
            default:
                break;
            }
        }
    }

    bool Window::ShouldClose() const { return m_ShouldClose; }

    uint32_t Window::GetWidth() const { return m_Width; }

    uint32_t Window::GetHeight() const { return m_Height; }

    SDL_Window *Window::GetNativeHandle() const { return m_Window; }

    void Window::SwapBuffers() { return; }

    void Window::ToggleVsync() { return; }

} // namespace axiom