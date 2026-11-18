/**
 * @file axiom/include/axiom/Input/GamepadCodes.h
 * @brief Declares the public interface for the gamepad codes.
 */

#pragma once

#include <SDL3/SDL.h>

namespace axiom {
    /**
     * @brief Provides the gamepad button type alias.
     */
    using GamepadButton = int;
    /**
     * @brief Provides the gamepad axis type alias.
     */
    using GamepadAxis = int;
    /**
     * @brief Provides the gamepad sensor type alias.
     */
    using GamepadSensor = int;

    namespace gamepad {
        /** @brief Maps the A gamepad button to its SDL input code. */
        constexpr GamepadButton a = SDL_GAMEPAD_BUTTON_SOUTH;
        /** @brief Maps the B gamepad button to its SDL input code. */
        constexpr GamepadButton b = SDL_GAMEPAD_BUTTON_EAST;
        /** @brief Maps the X gamepad button to its SDL input code. */
        constexpr GamepadButton x = SDL_GAMEPAD_BUTTON_WEST;
        /** @brief Maps the Y gamepad button to its SDL input code. */
        constexpr GamepadButton y = SDL_GAMEPAD_BUTTON_NORTH;

        /** @brief Maps cross to its input code. */
        constexpr GamepadButton cross = SDL_GAMEPAD_BUTTON_SOUTH;
        /** @brief Maps circle to its input code. */
        constexpr GamepadButton circle = SDL_GAMEPAD_BUTTON_EAST;
        /** @brief Maps square to its input code. */
        constexpr GamepadButton square = SDL_GAMEPAD_BUTTON_WEST;
        /** @brief Maps triangle to its input code. */
        constexpr GamepadButton triangle = SDL_GAMEPAD_BUTTON_NORTH;

        /** @brief Maps left bumper to its input code. */
        constexpr GamepadButton leftBumper = SDL_GAMEPAD_BUTTON_LEFT_SHOULDER;
        /** @brief Maps right bumper to its input code. */
        constexpr GamepadButton rightBumper = SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER;
        /** @brief Maps back to its input code. */
        constexpr GamepadButton back = SDL_GAMEPAD_BUTTON_BACK;
        /** @brief Maps start to its input code. */
        constexpr GamepadButton start = SDL_GAMEPAD_BUTTON_START;
        /** @brief Maps guide to its input code. */
        constexpr GamepadButton guide = SDL_GAMEPAD_BUTTON_GUIDE;
        /** @brief Maps left thumb to its input code. */
        constexpr GamepadButton leftThumb = SDL_GAMEPAD_BUTTON_LEFT_STICK;
        /** @brief Maps right thumb to its input code. */
        constexpr GamepadButton rightThumb = SDL_GAMEPAD_BUTTON_RIGHT_STICK;
        /** @brief Maps d pad up to its input code. */
        constexpr GamepadButton dPadUp = SDL_GAMEPAD_BUTTON_DPAD_UP;
        /** @brief Maps d pad right to its input code. */
        constexpr GamepadButton dPadRight = SDL_GAMEPAD_BUTTON_DPAD_RIGHT;
        /** @brief Maps d pad down to its input code. */
        constexpr GamepadButton dPadDown = SDL_GAMEPAD_BUTTON_DPAD_DOWN;
        /** @brief Maps d pad left to its input code. */
        constexpr GamepadButton dPadLeft = SDL_GAMEPAD_BUTTON_DPAD_LEFT;
        /** @brief Maps misc1 to its input code. */
        constexpr GamepadButton misc1 = SDL_GAMEPAD_BUTTON_MISC1;
        /** @brief Maps right stick to its input code. */
        constexpr GamepadButton rightStick = SDL_GAMEPAD_BUTTON_RIGHT_STICK;
        /** @brief Maps left stick to its input code. */
        constexpr GamepadButton leftStick = SDL_GAMEPAD_BUTTON_LEFT_STICK;
        /** @brief Maps touchpad to its input code. */
        constexpr GamepadButton touchpad = SDL_GAMEPAD_BUTTON_TOUCHPAD;

        /** @brief Maps left x to its input code. */
        constexpr GamepadAxis leftX = SDL_GAMEPAD_AXIS_LEFTX;
        /** @brief Maps left y to its input code. */
        constexpr GamepadAxis leftY = SDL_GAMEPAD_AXIS_LEFTY;
        /** @brief Maps right x to its input code. */
        constexpr GamepadAxis rightX = SDL_GAMEPAD_AXIS_RIGHTX;
        /** @brief Maps right y to its input code. */
        constexpr GamepadAxis rightY = SDL_GAMEPAD_AXIS_RIGHTY;
        /** @brief Maps left trigger to its input code. */
        constexpr GamepadAxis leftTrigger = SDL_GAMEPAD_AXIS_LEFT_TRIGGER;
        /** @brief Maps right trigger to its input code. */
        constexpr GamepadAxis rightTrigger = SDL_GAMEPAD_AXIS_RIGHT_TRIGGER;
    } // namespace Gamepad

    namespace gamepad_sensors {
        /** @brief Maps accelerometer to its input code. */
        constexpr GamepadSensor accelerometer = SDL_SENSOR_ACCEL;
        /** @brief Maps gyroscope to its input code. */
        constexpr GamepadSensor gyroscope = SDL_SENSOR_GYRO;
    } // namespace GamepadSensors
} // namespace axiom
