/**
 * @file axiom/src/platform/Window.cpp
 * @brief Implements the window module.
 */

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

    static bool sSdlInitialized = false;

    Window::Window(const Props &props, EventBus &eventBus)
        /**
         * @brief Performs the event bus operation.
         * @return The result of the operation.
         */
        : m_eventBus(eventBus) {
        init(props);
    }

    Window::~Window() { shutdown(); }

    void Window::init(const Props &props) {
        if (!sSdlInitialized) {
            if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD |
                          SDL_INIT_JOYSTICK | SDL_INIT_HAPTIC |
                          SDL_INIT_SENSOR | SDL_INIT_AUDIO))
                throw std::runtime_error(
                    std::string("Failed to initialize SDL3: ") +
                    SDL_GetError());

            sSdlInitialized = true;
        }

        SDL_WindowFlags flags = SDL_WINDOW_RESIZABLE;

        flags |= SDL_WINDOW_VULKAN;

        m_window =
            SDL_CreateWindow(props.title.c_str(), static_cast<int>(props.width),
                             static_cast<int>(props.height), flags);
        if (!m_window)
            throw std::runtime_error(
                std::string("Failed to create SDL3 window: ") + SDL_GetError());


        m_width = props.width;
        m_height = props.height;
    }

    void Window::shutdown() {
        Input::shutdown();

        if (m_window) {
            SDL_DestroyWindow(m_window);
            m_window = nullptr;
        }

        if (sSdlInitialized) {
            SDL_Quit();
            sSdlInitialized = false;
        }
    }

    void Window::pollEvents() {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (m_nativeEventHook && m_nativeEventHook(event))
                continue;
            switch (event.type) {
            case SDL_EVENT_QUIT: {
                m_shouldClose = true;
                WindowCloseEvent closeEvent;
                m_eventBus.publish(closeEvent);
                break;
            }
            case SDL_EVENT_WINDOW_CLOSE_REQUESTED: {
                if (event.window.windowID != SDL_GetWindowID(m_window))
                    break;

                m_shouldClose = true;
                WindowCloseEvent closeEvent;
                m_eventBus.publish(closeEvent);
                break;
            }
            case SDL_EVENT_WINDOW_RESIZED: {
                if (event.window.windowID != SDL_GetWindowID(m_window))
                    break;

                m_width = static_cast<uint32_t>(event.window.data1);
                m_height = static_cast<uint32_t>(event.window.data2);
                WindowResizeEvent resizeEvent{m_width, m_height};
                m_eventBus.publish(resizeEvent);
                break;
            }
            case SDL_EVENT_KEY_DOWN:
            case SDL_EVENT_KEY_UP: {
                KeyEvent keyEvent{static_cast<int>(event.key.key),
                                  static_cast<int>(event.key.scancode),
                                  event.type == SDL_EVENT_KEY_DOWN ? 1 : 0,
                                  static_cast<int>(event.key.mod),
                                  event.key.repeat ? 1 : 0};
                m_eventBus.publish(keyEvent);
                break;
            }
            case SDL_EVENT_MOUSE_BUTTON_DOWN:
            case SDL_EVENT_MOUSE_BUTTON_UP: {
                MouseButtonEvent mouseButtonEvent{
                    static_cast<int>(event.button.button),
                    event.type == SDL_EVENT_MOUSE_BUTTON_DOWN ? 1 : 0, 0};
                m_eventBus.publish(mouseButtonEvent);
                break;
            }
            case SDL_EVENT_MOUSE_MOTION: {
                MouseMoveEvent mouseMoveEvent{
                    static_cast<double>(event.motion.x),
                    static_cast<double>(event.motion.y)};
                m_eventBus.publish(mouseMoveEvent);
                break;
            }
            case SDL_EVENT_MOUSE_WHEEL: {
                Input::onMouseScroll(event.wheel.x, event.wheel.y);
                MouseScrollEvent scrollEvent{
                    static_cast<double>(event.wheel.x),
                    static_cast<double>(event.wheel.y)};
                m_eventBus.publish(scrollEvent);
                break;
            }
            case SDL_EVENT_TEXT_INPUT: {
                const char *text = event.text.text;
                if (text) {
                    while (*text) {
                        CharEvent charEvent{static_cast<unsigned int>(*text)};
                        m_eventBus.publish(charEvent);
                        ++text;
                    }
                }
                break;
            }
            case SDL_EVENT_DROP_FILE: {
                const char *path = event.drop.data;
                FileDropEvent dropEvent{path ? 1 : 0, &path};
                m_eventBus.publish(dropEvent);
                break;
            }
            case SDL_EVENT_GAMEPAD_ADDED:
            case SDL_EVENT_GAMEPAD_REMOVED:
                Input::onGamepadChanged();
                break;
            case SDL_EVENT_JOYSTICK_ADDED:
            case SDL_EVENT_JOYSTICK_REMOVED:
                Input::onJoystickChanged();
                break;
            default:
                break;
            }
        }
    }

    bool Window::shouldClose() const { return m_shouldClose; }

    uint32_t Window::getWidth() const { return m_width; }

    uint32_t Window::getHeight() const { return m_height; }

    SDL_Window *Window::getNativeHandle() const { return m_window; }

} // namespace axiom