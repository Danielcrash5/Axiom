/**
 * @file axiom/include/axiom/Input/JoystickCodes.h
 * @brief Declares the public interface for the joystick codes.
 */

#pragma once

#include <SDL3/SDL.h>

#include <cstdint>

namespace axiom {
    /**
     * @brief Provides the joystick button type alias.
     */
    using JoystickButton = int;
    /**
     * @brief Provides the joystick axis type alias.
     */
    using JoystickAxis = int;
    /**
     * @brief Provides the joystick hat index type alias.
     */
    using JoystickHatIndex = int;
    /**
     * @brief Provides the joystick hat type alias.
     */
    using JoystickHat = std::uint8_t;

    namespace joystick {
        /** @brief Maps axis x to its input code. */
        constexpr JoystickAxis axisX = 0;
        /** @brief Maps axis y to its input code. */
        constexpr JoystickAxis axisY = 1;
        /** @brief Maps axis z to its input code. */
        constexpr JoystickAxis axisZ = 2;
        /** @brief Maps axis rotation x to its input code. */
        constexpr JoystickAxis axisRotationX = 3;
        /** @brief Maps axis rotation y to its input code. */
        constexpr JoystickAxis axisRotationY = 4;
        /** @brief Maps axis rotation z to its input code. */
        constexpr JoystickAxis axisRotationZ = 5;
        /** @brief Maps axis slider0 to its input code. */
        constexpr JoystickAxis axisSlider0 = 6;
        /** @brief Maps axis slider1 to its input code. */
        constexpr JoystickAxis axisSlider1 = 7;
        /** @brief Maps axis dial to its input code. */
        constexpr JoystickAxis axisDial = 8;
        /** @brief Maps axis wheel to its input code. */
        constexpr JoystickAxis axisWheel = 9;

        /** @brief Maps stick x to its input code. */
        constexpr JoystickAxis stickX = axisX;
        /** @brief Maps stick y to its input code. */
        constexpr JoystickAxis stickY = axisY;
        /** @brief Maps throttle to its input code. */
        constexpr JoystickAxis throttle = axisZ;
        /** @brief Maps rudder to its input code. */
        constexpr JoystickAxis rudder = axisRotationZ;

        /** @brief Maps button1 to its input code. */
        constexpr JoystickButton button1 = 0;
        /** @brief Maps button2 to its input code. */
        constexpr JoystickButton button2 = 1;
        /** @brief Maps button3 to its input code. */
        constexpr JoystickButton button3 = 2;
        /** @brief Maps button4 to its input code. */
        constexpr JoystickButton button4 = 3;
        /** @brief Maps button5 to its input code. */
        constexpr JoystickButton button5 = 4;
        /** @brief Maps button6 to its input code. */
        constexpr JoystickButton button6 = 5;
        /** @brief Maps button7 to its input code. */
        constexpr JoystickButton button7 = 6;
        /** @brief Maps button8 to its input code. */
        constexpr JoystickButton button8 = 7;
        /** @brief Maps button9 to its input code. */
        constexpr JoystickButton button9 = 8;
        /** @brief Maps button10 to its input code. */
        constexpr JoystickButton button10 = 9;
        /** @brief Maps button11 to its input code. */
        constexpr JoystickButton button11 = 10;
        /** @brief Maps button12 to its input code. */
        constexpr JoystickButton button12 = 11;
        /** @brief Maps button13 to its input code. */
        constexpr JoystickButton button13 = 12;
        /** @brief Maps button14 to its input code. */
        constexpr JoystickButton button14 = 13;
        /** @brief Maps button15 to its input code. */
        constexpr JoystickButton button15 = 14;
        /** @brief Maps button16 to its input code. */
        constexpr JoystickButton button16 = 15;
        /** @brief Maps button17 to its input code. */
        constexpr JoystickButton button17 = 16;
        /** @brief Maps button18 to its input code. */
        constexpr JoystickButton button18 = 17;
        /** @brief Maps button19 to its input code. */
        constexpr JoystickButton button19 = 18;
        /** @brief Maps button20 to its input code. */
        constexpr JoystickButton button20 = 19;
        /** @brief Maps button21 to its input code. */
        constexpr JoystickButton button21 = 20;
        /** @brief Maps button22 to its input code. */
        constexpr JoystickButton button22 = 21;
        /** @brief Maps button23 to its input code. */
        constexpr JoystickButton button23 = 22;
        /** @brief Maps button24 to its input code. */
        constexpr JoystickButton button24 = 23;
        /** @brief Maps button25 to its input code. */
        constexpr JoystickButton button25 = 24;
        /** @brief Maps button26 to its input code. */
        constexpr JoystickButton button26 = 25;
        /** @brief Maps button27 to its input code. */
        constexpr JoystickButton button27 = 26;
        /** @brief Maps button28 to its input code. */
        constexpr JoystickButton button28 = 27;
        /** @brief Maps button29 to its input code. */
        constexpr JoystickButton button29 = 28;
        /** @brief Maps button30 to its input code. */
        constexpr JoystickButton button30 = 29;
        /** @brief Maps button31 to its input code. */
        constexpr JoystickButton button31 = 30;
        /** @brief Maps button32 to its input code. */
        constexpr JoystickButton button32 = 31;

        /** @brief Maps trigger to its input code. */
        constexpr JoystickButton trigger = button1;
        /** @brief Maps thumb to its input code. */
        constexpr JoystickButton thumb = button2;
        /** @brief Maps thumb2 to its input code. */
        constexpr JoystickButton thumb2 = button3;
        /** @brief Maps top to its input code. */
        constexpr JoystickButton top = button4;
        /** @brief Maps top2 to its input code. */
        constexpr JoystickButton top2 = button5;
        /** @brief Maps pinkie to its input code. */
        constexpr JoystickButton pinkie = button6;
        /** @brief Maps base1 to its input code. */
        constexpr JoystickButton base1 = button7;
        /** @brief Maps base2 to its input code. */
        constexpr JoystickButton base2 = button8;
        /** @brief Maps base3 to its input code. */
        constexpr JoystickButton base3 = button9;
        /** @brief Maps base4 to its input code. */
        constexpr JoystickButton base4 = button10;
        /** @brief Maps base5 to its input code. */
        constexpr JoystickButton base5 = button11;
        /** @brief Maps base6 to its input code. */
        constexpr JoystickButton base6 = button12;

        /** @brief Maps hat0 to its input code. */
        constexpr JoystickHatIndex hat0 = 0;
        /** @brief Maps hat1 to its input code. */
        constexpr JoystickHatIndex hat1 = 1;
        /** @brief Maps hat2 to its input code. */
        constexpr JoystickHatIndex hat2 = 2;
        /** @brief Maps hat3 to its input code. */
        constexpr JoystickHatIndex hat3 = 3;

        /** @brief Maps hat centered to its input code. */
        constexpr JoystickHat hatCentered = SDL_HAT_CENTERED;
        /** @brief Maps hat up to its input code. */
        constexpr JoystickHat hatUp = SDL_HAT_UP;
        /** @brief Maps hat right to its input code. */
        constexpr JoystickHat hatRight = SDL_HAT_RIGHT;
        /** @brief Maps hat down to its input code. */
        constexpr JoystickHat hatDown = SDL_HAT_DOWN;
        /** @brief Maps hat left to its input code. */
        constexpr JoystickHat hatLeft = SDL_HAT_LEFT;
        /** @brief Maps hat right up to its input code. */
        constexpr JoystickHat hatRightUp = SDL_HAT_RIGHTUP;
        /** @brief Maps hat right down to its input code. */
        constexpr JoystickHat hatRightDown = SDL_HAT_RIGHTDOWN;
        /** @brief Maps hat left up to its input code. */
        constexpr JoystickHat hatLeftUp = SDL_HAT_LEFTUP;
        /** @brief Maps hat left down to its input code. */
        constexpr JoystickHat hatLeftDown = SDL_HAT_LEFTDOWN;
    } // namespace Joystick
} // namespace axiom
