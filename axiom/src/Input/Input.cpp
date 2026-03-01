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

    static std::vector<KeyCode> s_ValidKeys = {
		Key::A, Key::B, Key::C, Key::D, Key::E, Key::F, Key::G, Key::H, Key::I, Key::J,
		Key::K, Key::L, Key::M, Key::N, Key::O, Key::P, Key::Q, Key::R, Key::S, Key::T,
		Key::U, Key::V, Key::W, Key::X, Key::Y, Key::Z,
		Key::Num0, Key::Num1, Key::Num2, Key::Num3, Key::Num4,
		Key::Num5, Key::Num6, Key::Num7, Key::Num8, Key::Num9,
		Key::F1, Key::F2, Key::F3, Key::F4, Key::F5, Key::F6,
		Key::F7, Key::F8, Key::F9, Key::F10, Key::F11, Key::F12,
		Key::Space, Key::Escape, Key::Enter, Key::Tab, Key::Backspace,
		Key::Insert, Key::Delete, Key::Right, Key::Left, Key::Down,
		Key::Up, Key::PageUp, Key::PageDown, Key::Home, Key::End,
		Key::CapsLock, Key::ScrollLock, Key::NumLock, Key::PrintScreen,
		Key::Pause, Key::Menu,
		Key::LeftShift, Key::LeftControl, Key::LeftAlt, Key::LeftSuper,
		Key::RightShift, Key::RightControl, Key::RightAlt, Key::RightSuper,
		Key::KP0, Key::KP1, Key::KP2, Key::KP3, Key::KP4,
        Key::KP5, Key::KP6, Key::KP7, Key::KP8, Key::KP9,
		Key::KPDecimal, Key::KPDivide, Key::KPMultiply, Key::KPSubtract, Key::KPAdd,
		Key::F1, Key::F2, Key::F3, Key::F4, Key::F5, Key::F6, Key::F7, Key::F8, Key::F9, Key::F10, Key::F11, Key::F12,
		Key::F13, Key::F14, Key::F15, Key::F16, Key::F17, Key::F18, Key::F19, Key::F20,
		Key::F21, Key::F22, Key::F23, Key::F24,
		Key::KPEnter, Key::KPEqual,
		Key::RightSuper, Key::RightAlt, Key::RightControl, Key::RightShift,
		Key::LeftSuper, Key::LeftAlt, Key::LeftControl, Key::LeftShift,
		Key::Menu, Key::Pause, Key::PrintScreen, Key::NumLock, Key::ScrollLock, Key::CapsLock,
		Key::End, Key::Home, Key::PageDown, Key::PageUp, Key::Up, Key::Down, Key::Left, Key::Right,
		Key::Insert, Key::Delete, Key::Backspace, Key::Tab, Key::Enter, Key::Escape, Key::Space
	};

    void Input::Init(void* window) {
        s_Window = (GLFWwindow*)window;
    }

    void Input::Update() {
        s_PreviousKeys = s_CurrentKeys;
        s_PreviousMouse = s_CurrentMouse;
        s_PreviousMousePosition = s_MousePosition;

        // Keyboard
        for (KeyCode key : s_ValidKeys)
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