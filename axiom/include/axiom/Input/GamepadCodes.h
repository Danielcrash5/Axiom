#pragma once

#include <SDL3/SDL.h>

namespace axiom {
    using GamepadButton = int;
    using GamepadAxis = int;
    using GamepadSensor = int;

    namespace Gamepad {
        constexpr GamepadButton A = SDL_GAMEPAD_BUTTON_SOUTH;
        constexpr GamepadButton B = SDL_GAMEPAD_BUTTON_EAST;
        constexpr GamepadButton X = SDL_GAMEPAD_BUTTON_WEST;
        constexpr GamepadButton Y = SDL_GAMEPAD_BUTTON_NORTH;

        constexpr GamepadButton Cross = SDL_GAMEPAD_BUTTON_SOUTH;
        constexpr GamepadButton Circle = SDL_GAMEPAD_BUTTON_EAST;
        constexpr GamepadButton Square = SDL_GAMEPAD_BUTTON_WEST;
        constexpr GamepadButton Triangle = SDL_GAMEPAD_BUTTON_NORTH;

        constexpr GamepadButton LeftBumper = SDL_GAMEPAD_BUTTON_LEFT_SHOULDER;
        constexpr GamepadButton RightBumper = SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER;
        constexpr GamepadButton Back = SDL_GAMEPAD_BUTTON_BACK;
        constexpr GamepadButton Start = SDL_GAMEPAD_BUTTON_START;
        constexpr GamepadButton Guide = SDL_GAMEPAD_BUTTON_GUIDE;
        constexpr GamepadButton LeftThumb = SDL_GAMEPAD_BUTTON_LEFT_STICK;
        constexpr GamepadButton RightThumb = SDL_GAMEPAD_BUTTON_RIGHT_STICK;
        constexpr GamepadButton DPadUp = SDL_GAMEPAD_BUTTON_DPAD_UP;
        constexpr GamepadButton DPadRight = SDL_GAMEPAD_BUTTON_DPAD_RIGHT;
        constexpr GamepadButton DPadDown = SDL_GAMEPAD_BUTTON_DPAD_DOWN;
        constexpr GamepadButton DPadLeft = SDL_GAMEPAD_BUTTON_DPAD_LEFT;
        constexpr GamepadButton Share = SDL_GAMEPAD_BUTTON_MISC1;
        constexpr GamepadButton Touchpad = SDL_GAMEPAD_BUTTON_TOUCHPAD;

        constexpr GamepadAxis LeftX = SDL_GAMEPAD_AXIS_LEFTX;
        constexpr GamepadAxis LeftY = SDL_GAMEPAD_AXIS_LEFTY;
        constexpr GamepadAxis RightX = SDL_GAMEPAD_AXIS_RIGHTX;
        constexpr GamepadAxis RightY = SDL_GAMEPAD_AXIS_RIGHTY;
        constexpr GamepadAxis LeftTrigger = SDL_GAMEPAD_AXIS_LEFT_TRIGGER;
        constexpr GamepadAxis RightTrigger = SDL_GAMEPAD_AXIS_RIGHT_TRIGGER;
    }

    namespace GamepadSensors {
        constexpr GamepadSensor Accelerometer = SDL_SENSOR_ACCEL;
        constexpr GamepadSensor Gyroscope = SDL_SENSOR_GYRO;
    }
}
