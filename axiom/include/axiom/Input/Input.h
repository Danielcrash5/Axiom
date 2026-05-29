#pragma once

#include <glm/glm.hpp>

#include <array>
#include <cstdint>
#include <string>

#include "KeyCodes.h"
#include "MouseCodes.h"
#include "GamepadCodes.h"

struct SDL_Window;
struct SDL_Gamepad;

namespace axiom {

    class Input {
    public:
        static void Init(void* window);
        static void Shutdown();
        static void Update();

        // ================= Keyboard =================
        static bool IsKeyPressed(KeyCode key);
        static bool IsKeyJustPressed(KeyCode key);
        static bool IsKeyReleased(KeyCode key);

        // ================= Mouse =================
        static bool IsMousePressed(MouseCode button);
        static bool IsMouseJustPressed(MouseCode button);
        static bool IsMouseReleased(MouseCode button);

        static glm::vec2 GetMousePosition();
        static glm::vec2 GetMouseDelta();
        static glm::vec2 GetMouseScroll();
        static glm::vec2 GetMouseScrollDelta();

        // ================= Gamepad =================
        static bool IsGamepadConnected(int id = 0);
        static std::string GetGamepadName(int id = 0);

        static bool IsGamepadPressed(GamepadButton button, int id = 0);

        static glm::vec2 GetGamepadLeftStick(int id = 0);
        static glm::vec2 GetGamepadRightStick(int id = 0);

        static float GetGamepadAxis(GamepadAxis axis, int id = 0);

        static float GetGamepadLeftTrigger(int id = 0);
        static float GetGamepadRightTrigger(int id = 0);

        static bool RumbleGamepad(uint16_t lowFrequency, uint16_t highFrequency, uint32_t durationMs, int id = 0);
        static bool RumbleGamepadTriggers(uint16_t leftTrigger, uint16_t rightTrigger, uint32_t durationMs, int id = 0);
        static bool SetGamepadLED(uint8_t red, uint8_t green, uint8_t blue, int id = 0);
        static bool SetGamepadSensorEnabled(GamepadSensor sensor, bool enabled, int id = 0);
        static bool GetGamepadSensorData(GamepadSensor sensor, float* data, int valueCount, int id = 0);
        static int GetGamepadTouchpadCount(int id = 0);
        static int GetGamepadTouchpadFingerCount(int touchpad = 0, int id = 0);
        static bool GetGamepadTouchpadFinger(int touchpad, int finger, bool& down, float& x, float& y, float& pressure, int id = 0);

    private:
        static SDL_Window* s_Window;

        static std::array<bool, SDL_SCANCODE_COUNT> s_CurrentKeys;
        static std::array<bool, SDL_SCANCODE_COUNT> s_PreviousKeys;

        static std::array<bool, 9> s_CurrentMouse;
        static std::array<bool, 9> s_PreviousMouse;

        static std::array<SDL_Gamepad*, 4> s_Gamepads;

        static glm::vec2 s_MousePosition;
        static glm::vec2 s_PreviousMousePosition;
        static glm::vec2 s_MouseScroll;
        static glm::vec2 s_PendingMouseScroll;

        static float ApplyDeadzone(float value, float deadzone = 0.1f);
        static SDL_Gamepad* GetGamepad(int id);
        static void RefreshGamepads();

    public:
        // Internal hooks used by the window backend.
        static void OnMouseScroll(double xOffset, double yOffset);
        static void OnGamepadChanged();
    };

}
