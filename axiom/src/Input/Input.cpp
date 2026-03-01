#include "axiom/input/Input.h"
#include <GLFW/glfw3.h>
#include <cmath>

namespace axiom {

    GLFWwindow* Input::s_Window = nullptr;

    std::array<bool, GLFW_KEY_LAST + 1> Input::s_CurrentKeys {};
    std::array<bool, GLFW_KEY_LAST + 1> Input::s_PreviousKeys {};

    std::array<bool, GLFW_MOUSE_BUTTON_LAST + 1> Input::s_CurrentMouse = {false};
    std::array<bool, GLFW_MOUSE_BUTTON_LAST + 1> Input::s_PreviousMouse = {false};

    glm::vec2 Input::s_MousePosition {};
    glm::vec2 Input::s_PreviousMousePosition {};

    void Input::Init(void* window) {
        s_Window = (GLFWwindow*)window;
    }

    void Input::Update() {
        s_PreviousKeys = s_CurrentKeys;
        s_PreviousMouse = s_CurrentMouse;
        s_PreviousMousePosition = s_MousePosition;

        // Keyboard
        for (int key = 0; key <= GLFW_KEY_LAST; ++key)
            s_CurrentKeys[key] = glfwGetKey(s_Window, key) == GLFW_PRESS;

        // Mouse
        for (int button = 0; button <= GLFW_MOUSE_BUTTON_LAST; ++button)
            s_CurrentMouse[button] = glfwGetMouseButton(s_Window, button) == GLFW_PRESS;

        double x, y;
        glfwGetCursorPos(s_Window, &x, &y);
        s_MousePosition = glm::vec2((float)x, (float)y);
    }

    float Input::ApplyDeadzone(float value, float deadzone) {
        return std::abs(value) < deadzone ? 0.0f : value;
    }

    // ================= Keyboard =================

    bool Input::IsKeyPressed(KeyCode key) {
        return s_CurrentKeys[key];
    }

    bool Input::IsKeyJustPressed(KeyCode key) {
        return s_CurrentKeys[key] && !s_PreviousKeys[key];
    }

    bool Input::IsKeyReleased(KeyCode key) {
        return !s_CurrentKeys[key] && s_PreviousKeys[key];
    }

    // ================= Mouse =================

    bool Input::IsMousePressed(MouseCode button) {
        return s_CurrentMouse[button];
    }

    bool Input::IsMouseJustPressed(MouseCode button) {
        return s_CurrentMouse[button] && !s_PreviousMouse[button];
    }

    bool Input::IsMouseReleased(MouseCode button) {
        return !s_CurrentMouse[button] && s_PreviousMouse[button];
    }

    glm::vec2 Input::GetMousePosition() {
        return s_MousePosition;
    }

    glm::vec2 Input::GetMouseDelta() {
        return s_MousePosition - s_PreviousMousePosition;
    }

    // ================= Gamepad =================

    bool Input::IsGamepadConnected(int id) {
        return glfwJoystickIsGamepad(id);
    }

    bool Input::IsGamepadPressed(GamepadButton button, int id) {
        if (!IsGamepadConnected(id))
            return false;

        GLFWgamepadstate state;
        glfwGetGamepadState(id, &state);

        return state.buttons[button] == GLFW_PRESS;
    }

    glm::vec2 Input::GetGamepadLeftStick(int id) {
        if (!IsGamepadConnected(id))
            return glm::vec2(0.0f);

        GLFWgamepadstate state;
        glfwGetGamepadState(id, &state);

        return glm::vec2(
            ApplyDeadzone(state.axes[GLFW_GAMEPAD_AXIS_LEFT_X]),
            ApplyDeadzone(state.axes[GLFW_GAMEPAD_AXIS_LEFT_Y])
        );
    }

    glm::vec2 Input::GetGamepadRightStick(int id) {
        if (!IsGamepadConnected(id))
            return glm::vec2(0.0f);

        GLFWgamepadstate state;
        glfwGetGamepadState(id, &state);

        return glm::vec2(
            ApplyDeadzone(state.axes[GLFW_GAMEPAD_AXIS_RIGHT_X]),
            ApplyDeadzone(state.axes[GLFW_GAMEPAD_AXIS_RIGHT_Y])
        );
    }

    // Implementation: Input.cpp
    float Input::GetGamepadAxis(int axis, int id) {
        if (!IsGamepadConnected(id))
            return 0.0f;

        GLFWgamepadstate state;
        if (!glfwGetGamepadState(id, &state))
            return 0.0f;

        // Achsen ab 0 bis GLFW_GAMEPAD_AXIS_LAST
        if (axis < 0 || axis > GLFW_GAMEPAD_AXIS_LAST)
            return 0.0f;

        return ApplyDeadzone(state.axes[axis]);
    }

    float Input::GetGamepadLeftTrigger(int id) {
        if (!IsGamepadConnected(id))
            return 0.0f;

        GLFWgamepadstate state;
        glfwGetGamepadState(id, &state);

        return ApplyDeadzone(state.axes[GLFW_GAMEPAD_AXIS_LEFT_TRIGGER]);
    }

    float Input::GetGamepadRightTrigger(int id) {
        if (!IsGamepadConnected(id))
            return 0.0f;

        GLFWgamepadstate state;
        glfwGetGamepadState(id, &state);

        return ApplyDeadzone(state.axes[GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER]);
    }

}