#include "axiom/input/Input.h"

#include <SDL3/SDL.h>

#include <algorithm>
#include <cmath>

namespace axiom {

    SDL_Window* Input::s_Window = nullptr;

    std::array<bool, SDL_SCANCODE_COUNT> Input::s_CurrentKeys {};
    std::array<bool, SDL_SCANCODE_COUNT> Input::s_PreviousKeys {};

    std::array<bool, 9> Input::s_CurrentMouse {};
    std::array<bool, 9> Input::s_PreviousMouse {};

    std::array<SDL_Gamepad*, 4> Input::s_Gamepads {};

    glm::vec2 Input::s_MousePosition {};
    glm::vec2 Input::s_PreviousMousePosition {};
    glm::vec2 Input::s_MouseScroll {};
    glm::vec2 Input::s_PendingMouseScroll {};

    void Input::Init(void* window) {
        s_Window = static_cast<SDL_Window*>(window);
        RefreshGamepads();
    }

    void Input::Shutdown() {
        for (SDL_Gamepad*& gamepad : s_Gamepads) {
            if (gamepad) {
                SDL_CloseGamepad(gamepad);
                gamepad = nullptr;
            }
        }

        s_Window = nullptr;
    }

    void Input::Update() {
        s_PreviousKeys = s_CurrentKeys;
        s_PreviousMouse = s_CurrentMouse;
        s_PreviousMousePosition = s_MousePosition;
        s_MouseScroll = s_PendingMouseScroll;
        s_PendingMouseScroll = glm::vec2(0.0f);

        int keyCount = 0;
        const bool* keyboardState = SDL_GetKeyboardState(&keyCount);
        const int copyCount = std::min(keyCount, static_cast<int>(SDL_SCANCODE_COUNT));
        for (int key = 0; key < copyCount; ++key)
            s_CurrentKeys[key] = keyboardState[key];
        for (int key = copyCount; key < SDL_SCANCODE_COUNT; ++key)
            s_CurrentKeys[key] = false;

        float mouseX = 0.0f;
        float mouseY = 0.0f;
        const SDL_MouseButtonFlags mouseState = SDL_GetMouseState(&mouseX, &mouseY);
        s_MousePosition = glm::vec2(mouseX, mouseY);

        for (size_t button = 0; button < s_CurrentMouse.size(); ++button)
            s_CurrentMouse[button] = (mouseState & SDL_BUTTON_MASK(static_cast<int>(button))) != 0;

        SDL_UpdateGamepads();
    }

    float Input::ApplyDeadzone(float value, float deadzone) {
        return std::abs(value) < deadzone ? 0.0f : value;
    }

    bool Input::IsKeyPressed(KeyCode key) {
        return key >= 0 && key < SDL_SCANCODE_COUNT && s_CurrentKeys[key];
    }

    bool Input::IsKeyJustPressed(KeyCode key) {
        return key >= 0 && key < SDL_SCANCODE_COUNT && s_CurrentKeys[key] && !s_PreviousKeys[key];
    }

    bool Input::IsKeyReleased(KeyCode key) {
        return key >= 0 && key < SDL_SCANCODE_COUNT && !s_CurrentKeys[key] && s_PreviousKeys[key];
    }

    bool Input::IsMousePressed(MouseCode button) {
        return button >= 0 && static_cast<size_t>(button) < s_CurrentMouse.size() && s_CurrentMouse[button];
    }

    bool Input::IsMouseJustPressed(MouseCode button) {
        return button >= 0 && static_cast<size_t>(button) < s_CurrentMouse.size() &&
               s_CurrentMouse[button] && !s_PreviousMouse[button];
    }

    bool Input::IsMouseReleased(MouseCode button) {
        return button >= 0 && static_cast<size_t>(button) < s_CurrentMouse.size() &&
               !s_CurrentMouse[button] && s_PreviousMouse[button];
    }

    glm::vec2 Input::GetMousePosition() {
        return s_MousePosition;
    }

    glm::vec2 Input::GetMouseDelta() {
        return s_MousePosition - s_PreviousMousePosition;
    }

    glm::vec2 Input::GetMouseScroll() {
        return s_MouseScroll;
    }

    glm::vec2 Input::GetMouseScrollDelta() {
        return s_MouseScroll;
    }

    void Input::OnMouseScroll(double xOffset, double yOffset) {
        s_PendingMouseScroll += glm::vec2(static_cast<float>(xOffset), static_cast<float>(yOffset));
    }

    void Input::RefreshGamepads() {
        for (SDL_Gamepad*& gamepad : s_Gamepads) {
            if (gamepad) {
                SDL_CloseGamepad(gamepad);
                gamepad = nullptr;
            }
        }

        int count = 0;
        SDL_JoystickID* ids = SDL_GetGamepads(&count);
        if (!ids)
            return;

        const int openCount = std::min<int>(count, static_cast<int>(s_Gamepads.size()));
        for (int i = 0; i < openCount; ++i)
            s_Gamepads[i] = SDL_OpenGamepad(ids[i]);

        SDL_free(ids);
    }

    void Input::OnGamepadChanged() {
        RefreshGamepads();
    }

    SDL_Gamepad* Input::GetGamepad(int id) {
        if (id < 0 || static_cast<size_t>(id) >= s_Gamepads.size())
            return nullptr;

        if (!s_Gamepads[id])
            RefreshGamepads();

        return s_Gamepads[id];
    }

    bool Input::IsGamepadConnected(int id) {
        return GetGamepad(id) != nullptr;
    }

    std::string Input::GetGamepadName(int id) {
        SDL_Gamepad* gamepad = GetGamepad(id);
        if (!gamepad)
            return {};

        const char* name = SDL_GetGamepadName(gamepad);
        return name ? name : "";
    }

    bool Input::IsGamepadPressed(GamepadButton button, int id) {
        SDL_Gamepad* gamepad = GetGamepad(id);
        if (!gamepad)
            return false;

        return SDL_GetGamepadButton(gamepad, static_cast<SDL_GamepadButton>(button));
    }

    float Input::GetGamepadAxis(GamepadAxis axis, int id) {
        SDL_Gamepad* gamepad = GetGamepad(id);
        if (!gamepad)
            return 0.0f;

        const auto sdlAxis = static_cast<SDL_GamepadAxis>(axis);
        const float value = static_cast<float>(SDL_GetGamepadAxis(gamepad, sdlAxis));
        const float normalizer = sdlAxis == SDL_GAMEPAD_AXIS_LEFT_TRIGGER || sdlAxis == SDL_GAMEPAD_AXIS_RIGHT_TRIGGER
            ? static_cast<float>(SDL_JOYSTICK_AXIS_MAX)
            : 32768.0f;

        return ApplyDeadzone(glm::clamp(value / normalizer, -1.0f, 1.0f));
    }

    glm::vec2 Input::GetGamepadLeftStick(int id) {
        return glm::vec2(
            GetGamepadAxis(SDL_GAMEPAD_AXIS_LEFTX, id),
            GetGamepadAxis(SDL_GAMEPAD_AXIS_LEFTY, id)
        );
    }

    glm::vec2 Input::GetGamepadRightStick(int id) {
        return glm::vec2(
            GetGamepadAxis(SDL_GAMEPAD_AXIS_RIGHTX, id),
            GetGamepadAxis(SDL_GAMEPAD_AXIS_RIGHTY, id)
        );
    }

    float Input::GetGamepadLeftTrigger(int id) {
        return GetGamepadAxis(SDL_GAMEPAD_AXIS_LEFT_TRIGGER, id);
    }

    float Input::GetGamepadRightTrigger(int id) {
        return GetGamepadAxis(SDL_GAMEPAD_AXIS_RIGHT_TRIGGER, id);
    }

    bool Input::RumbleGamepad(uint16_t lowFrequency, uint16_t highFrequency, uint32_t durationMs, int id) {
        SDL_Gamepad* gamepad = GetGamepad(id);
        return gamepad && SDL_RumbleGamepad(gamepad, lowFrequency, highFrequency, durationMs);
    }

    bool Input::RumbleGamepadTriggers(uint16_t leftTrigger, uint16_t rightTrigger, uint32_t durationMs, int id) {
        SDL_Gamepad* gamepad = GetGamepad(id);
        return gamepad && SDL_RumbleGamepadTriggers(gamepad, leftTrigger, rightTrigger, durationMs);
    }

    bool Input::SetGamepadLED(uint8_t red, uint8_t green, uint8_t blue, int id) {
        SDL_Gamepad* gamepad = GetGamepad(id);
        return gamepad && SDL_SetGamepadLED(gamepad, red, green, blue);
    }

    bool Input::SetGamepadSensorEnabled(GamepadSensor sensor, bool enabled, int id) {
        SDL_Gamepad* gamepad = GetGamepad(id);
        return gamepad && SDL_SetGamepadSensorEnabled(gamepad, static_cast<SDL_SensorType>(sensor), enabled);
    }

    bool Input::GetGamepadSensorData(GamepadSensor sensor, float* data, int valueCount, int id) {
        SDL_Gamepad* gamepad = GetGamepad(id);
        return gamepad && data && valueCount > 0 &&
               SDL_GetGamepadSensorData(gamepad, static_cast<SDL_SensorType>(sensor), data, valueCount);
    }

}
