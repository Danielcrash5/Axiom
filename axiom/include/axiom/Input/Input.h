#pragma once

#include <glm/glm.hpp>
#include <array>
#include "KeyCodes.h"
#include "MouseCodes.h"
#include "GamepadCodes.h"
#include <GLFW/glfw3.h>

namespace axiom {

    class Input {
    public:
        static void Init(void* window);
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
        static bool IsGamepadConnected(int id = GLFW_JOYSTICK_1);

        static bool IsGamepadPressed(GamepadButton button, int id = GLFW_JOYSTICK_1);

        static glm::vec2 GetGamepadLeftStick(int id = GLFW_JOYSTICK_1);
        static glm::vec2 GetGamepadRightStick(int id = GLFW_JOYSTICK_1);

        static float GetGamepadAxis(int axis, int id = GLFW_JOYSTICK_1);

        static float GetGamepadLeftTrigger(int id = GLFW_JOYSTICK_1);
        static float GetGamepadRightTrigger(int id = GLFW_JOYSTICK_1);

    private:
        static GLFWwindow* s_Window;

        static std::array<bool, GLFW_KEY_LAST + 1> s_CurrentKeys;
        static std::array<bool, GLFW_KEY_LAST + 1> s_PreviousKeys;

        static std::array<bool, GLFW_MOUSE_BUTTON_LAST + 1> s_CurrentMouse;
        static std::array<bool, GLFW_MOUSE_BUTTON_LAST + 1> s_PreviousMouse;

        static glm::vec2 s_MousePosition;
        static glm::vec2 s_PreviousMousePosition;
        static glm::vec2 s_MouseScroll;
        static glm::vec2 s_PendingMouseScroll;

        static float ApplyDeadzone(float value, float deadzone = 0.1f);

    public:
        // Internal hook used by the window backend to forward GLFW scroll input.
        static void OnMouseScroll(double xOffset, double yOffset);
    };

}
