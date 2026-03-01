#pragma once

#include <GLFW/glfw3.h>

namespace axiom {

    using GamepadButton = int;
    using GamepadAxis = int;

    namespace Gamepad {
        constexpr GamepadButton A = GLFW_GAMEPAD_BUTTON_A;
        constexpr GamepadButton B = GLFW_GAMEPAD_BUTTON_B;
        constexpr GamepadButton X = GLFW_GAMEPAD_BUTTON_X;
        constexpr GamepadButton Y = GLFW_GAMEPAD_BUTTON_Y;

		constexpr GamepadButton Back = GLFW_GAMEPAD_BUTTON_BACK;
		constexpr GamepadButton Guide = GLFW_GAMEPAD_BUTTON_GUIDE;
		constexpr GamepadButton Start = GLFW_GAMEPAD_BUTTON_START;

		constexpr GamepadButton LeftThumb = GLFW_GAMEPAD_BUTTON_LEFT_THUMB;
		constexpr GamepadButton RightThumb = GLFW_GAMEPAD_BUTTON_RIGHT_THUMB;

		constexpr GamepadButton DPadUp = GLFW_GAMEPAD_BUTTON_DPAD_UP;
		constexpr GamepadButton DPadRight = GLFW_GAMEPAD_BUTTON_DPAD_RIGHT;
		constexpr GamepadButton DPadDown = GLFW_GAMEPAD_BUTTON_DPAD_DOWN;
		constexpr GamepadButton DPadLeft = GLFW_GAMEPAD_BUTTON_DPAD_LEFT;

		constexpr GamepadButton Cross = A;
		constexpr GamepadButton Circle = B;
		constexpr GamepadButton Square = X;
		constexpr GamepadButton Triangle = Y;

        constexpr GamepadButton LeftBumper = GLFW_GAMEPAD_BUTTON_LEFT_BUMPER;
        constexpr GamepadButton RightBumper = GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER;

        constexpr GamepadAxis LeftX = GLFW_GAMEPAD_AXIS_LEFT_X;
        constexpr GamepadAxis LeftY = GLFW_GAMEPAD_AXIS_LEFT_Y;
        constexpr GamepadAxis RightX = GLFW_GAMEPAD_AXIS_RIGHT_X;
        constexpr GamepadAxis RightY = GLFW_GAMEPAD_AXIS_RIGHT_Y;

        constexpr GamepadAxis LeftTrigger = GLFW_GAMEPAD_AXIS_LEFT_TRIGGER;
        constexpr GamepadAxis RightTrigger = GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER;
    }

}