#pragma once

#include <SDL3/SDL.h>

#include <cstdint>

namespace axiom {
using JoystickButton = int;
using JoystickAxis = int;
using JoystickHatIndex = int;
using JoystickHat = std::uint8_t;

namespace Joystick {
constexpr JoystickAxis AxisX = 0;
constexpr JoystickAxis AxisY = 1;
constexpr JoystickAxis AxisZ = 2;
constexpr JoystickAxis AxisRotationX = 3;
constexpr JoystickAxis AxisRotationY = 4;
constexpr JoystickAxis AxisRotationZ = 5;
constexpr JoystickAxis AxisSlider0 = 6;
constexpr JoystickAxis AxisSlider1 = 7;
constexpr JoystickAxis AxisDial = 8;
constexpr JoystickAxis AxisWheel = 9;

constexpr JoystickAxis StickX = AxisX;
constexpr JoystickAxis StickY = AxisY;
constexpr JoystickAxis Throttle = AxisZ;
constexpr JoystickAxis Rudder = AxisRotationZ;

constexpr JoystickButton Button1 = 0;
constexpr JoystickButton Button2 = 1;
constexpr JoystickButton Button3 = 2;
constexpr JoystickButton Button4 = 3;
constexpr JoystickButton Button5 = 4;
constexpr JoystickButton Button6 = 5;
constexpr JoystickButton Button7 = 6;
constexpr JoystickButton Button8 = 7;
constexpr JoystickButton Button9 = 8;
constexpr JoystickButton Button10 = 9;
constexpr JoystickButton Button11 = 10;
constexpr JoystickButton Button12 = 11;
constexpr JoystickButton Button13 = 12;
constexpr JoystickButton Button14 = 13;
constexpr JoystickButton Button15 = 14;
constexpr JoystickButton Button16 = 15;
constexpr JoystickButton Button17 = 16;
constexpr JoystickButton Button18 = 17;
constexpr JoystickButton Button19 = 18;
constexpr JoystickButton Button20 = 19;
constexpr JoystickButton Button21 = 20;
constexpr JoystickButton Button22 = 21;
constexpr JoystickButton Button23 = 22;
constexpr JoystickButton Button24 = 23;
constexpr JoystickButton Button25 = 24;
constexpr JoystickButton Button26 = 25;
constexpr JoystickButton Button27 = 26;
constexpr JoystickButton Button28 = 27;
constexpr JoystickButton Button29 = 28;
constexpr JoystickButton Button30 = 29;
constexpr JoystickButton Button31 = 30;
constexpr JoystickButton Button32 = 31;

constexpr JoystickButton Trigger = Button1;
constexpr JoystickButton Thumb = Button2;
constexpr JoystickButton Thumb2 = Button3;
constexpr JoystickButton Top = Button4;
constexpr JoystickButton Top2 = Button5;
constexpr JoystickButton Pinkie = Button6;
constexpr JoystickButton Base1 = Button7;
constexpr JoystickButton Base2 = Button8;
constexpr JoystickButton Base3 = Button9;
constexpr JoystickButton Base4 = Button10;
constexpr JoystickButton Base5 = Button11;
constexpr JoystickButton Base6 = Button12;

constexpr JoystickHatIndex Hat0 = 0;
constexpr JoystickHatIndex Hat1 = 1;
constexpr JoystickHatIndex Hat2 = 2;
constexpr JoystickHatIndex Hat3 = 3;

constexpr JoystickHat HatCentered = SDL_HAT_CENTERED;
constexpr JoystickHat HatUp = SDL_HAT_UP;
constexpr JoystickHat HatRight = SDL_HAT_RIGHT;
constexpr JoystickHat HatDown = SDL_HAT_DOWN;
constexpr JoystickHat HatLeft = SDL_HAT_LEFT;
constexpr JoystickHat HatRightUp = SDL_HAT_RIGHTUP;
constexpr JoystickHat HatRightDown = SDL_HAT_RIGHTDOWN;
constexpr JoystickHat HatLeftUp = SDL_HAT_LEFTUP;
constexpr JoystickHat HatLeftDown = SDL_HAT_LEFTDOWN;
} // namespace Joystick
} // namespace axiom
