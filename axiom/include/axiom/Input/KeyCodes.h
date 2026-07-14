#pragma once

#include <SDL3/SDL.h>

namespace axiom {
    using KeyCode = int;

    namespace Key {
        constexpr KeyCode A = SDL_SCANCODE_A;
        constexpr KeyCode B = SDL_SCANCODE_B;
        constexpr KeyCode C = SDL_SCANCODE_C;
        constexpr KeyCode D = SDL_SCANCODE_D;
        constexpr KeyCode E = SDL_SCANCODE_E;
        constexpr KeyCode F = SDL_SCANCODE_F;
        constexpr KeyCode G = SDL_SCANCODE_G;
        constexpr KeyCode H = SDL_SCANCODE_H;
        constexpr KeyCode I = SDL_SCANCODE_I;
        constexpr KeyCode J = SDL_SCANCODE_J;
        constexpr KeyCode K = SDL_SCANCODE_K;
        constexpr KeyCode L = SDL_SCANCODE_L;
        constexpr KeyCode M = SDL_SCANCODE_M;
        constexpr KeyCode N = SDL_SCANCODE_N;
        constexpr KeyCode O = SDL_SCANCODE_O;
        constexpr KeyCode P = SDL_SCANCODE_P;
        constexpr KeyCode Q = SDL_SCANCODE_Q;
        constexpr KeyCode R = SDL_SCANCODE_R;
        constexpr KeyCode S = SDL_SCANCODE_S;
        constexpr KeyCode T = SDL_SCANCODE_T;
        constexpr KeyCode U = SDL_SCANCODE_U;
        constexpr KeyCode V = SDL_SCANCODE_V;
        constexpr KeyCode W = SDL_SCANCODE_W;
        constexpr KeyCode X = SDL_SCANCODE_X;
        constexpr KeyCode Y = SDL_SCANCODE_Y;
        constexpr KeyCode Z = SDL_SCANCODE_Z;

        constexpr KeyCode Num0 = SDL_SCANCODE_0;
        constexpr KeyCode Num1 = SDL_SCANCODE_1;
        constexpr KeyCode Num2 = SDL_SCANCODE_2;
        constexpr KeyCode Num3 = SDL_SCANCODE_3;
        constexpr KeyCode Num4 = SDL_SCANCODE_4;
        constexpr KeyCode Num5 = SDL_SCANCODE_5;
        constexpr KeyCode Num6 = SDL_SCANCODE_6;
        constexpr KeyCode Num7 = SDL_SCANCODE_7;
        constexpr KeyCode Num8 = SDL_SCANCODE_8;
        constexpr KeyCode Num9 = SDL_SCANCODE_9;

        constexpr KeyCode F1 = SDL_SCANCODE_F1;
        constexpr KeyCode F2 = SDL_SCANCODE_F2;
        constexpr KeyCode F3 = SDL_SCANCODE_F3;
        constexpr KeyCode F4 = SDL_SCANCODE_F4;
        constexpr KeyCode F5 = SDL_SCANCODE_F5;
        constexpr KeyCode F6 = SDL_SCANCODE_F6;
        constexpr KeyCode F7 = SDL_SCANCODE_F7;
        constexpr KeyCode F8 = SDL_SCANCODE_F8;
        constexpr KeyCode F9 = SDL_SCANCODE_F9;
        constexpr KeyCode F10 = SDL_SCANCODE_F10;
        constexpr KeyCode F11 = SDL_SCANCODE_F11;
        constexpr KeyCode F12 = SDL_SCANCODE_F12;
        constexpr KeyCode F13 = SDL_SCANCODE_F13;
        constexpr KeyCode F14 = SDL_SCANCODE_F14;
        constexpr KeyCode F15 = SDL_SCANCODE_F15;
        constexpr KeyCode F16 = SDL_SCANCODE_F16;
        constexpr KeyCode F17 = SDL_SCANCODE_F17;
        constexpr KeyCode F18 = SDL_SCANCODE_F18;
        constexpr KeyCode F19 = SDL_SCANCODE_F19;
        constexpr KeyCode F20 = SDL_SCANCODE_F20;
        constexpr KeyCode F21 = SDL_SCANCODE_F21;
        constexpr KeyCode F22 = SDL_SCANCODE_F22;
        constexpr KeyCode F23 = SDL_SCANCODE_F23;
        constexpr KeyCode F24 = SDL_SCANCODE_F24;

        constexpr KeyCode Space = SDL_SCANCODE_SPACE;
        constexpr KeyCode Escape = SDL_SCANCODE_ESCAPE;
        constexpr KeyCode Enter = SDL_SCANCODE_RETURN;
        constexpr KeyCode Tab = SDL_SCANCODE_TAB;
        constexpr KeyCode Backspace = SDL_SCANCODE_BACKSPACE;
        constexpr KeyCode Insert = SDL_SCANCODE_INSERT;
        constexpr KeyCode Delete = SDL_SCANCODE_DELETE;
        constexpr KeyCode Right = SDL_SCANCODE_RIGHT;
        constexpr KeyCode Left = SDL_SCANCODE_LEFT;
        constexpr KeyCode Down = SDL_SCANCODE_DOWN;
        constexpr KeyCode Up = SDL_SCANCODE_UP;
        constexpr KeyCode PageUp = SDL_SCANCODE_PAGEUP;
        constexpr KeyCode PageDown = SDL_SCANCODE_PAGEDOWN;
        constexpr KeyCode Home = SDL_SCANCODE_HOME;
        constexpr KeyCode End = SDL_SCANCODE_END;
        constexpr KeyCode CapsLock = SDL_SCANCODE_CAPSLOCK;
        constexpr KeyCode ScrollLock = SDL_SCANCODE_SCROLLLOCK;
        constexpr KeyCode NumLock = SDL_SCANCODE_NUMLOCKCLEAR;
        constexpr KeyCode PrintScreen = SDL_SCANCODE_PRINTSCREEN;
        constexpr KeyCode Pause = SDL_SCANCODE_PAUSE;
        constexpr KeyCode Menu = SDL_SCANCODE_MENU;

        constexpr KeyCode LeftShift = SDL_SCANCODE_LSHIFT;
        constexpr KeyCode LeftControl = SDL_SCANCODE_LCTRL;
        constexpr KeyCode LeftAlt = SDL_SCANCODE_LALT;
        constexpr KeyCode LeftSuper = SDL_SCANCODE_LGUI;
        constexpr KeyCode RightShift = SDL_SCANCODE_RSHIFT;
        constexpr KeyCode RightControl = SDL_SCANCODE_RCTRL;
        constexpr KeyCode RightAlt = SDL_SCANCODE_RALT;
        constexpr KeyCode RightSuper = SDL_SCANCODE_RGUI;

        constexpr KeyCode KP0 = SDL_SCANCODE_KP_0;
        constexpr KeyCode KP1 = SDL_SCANCODE_KP_1;
        constexpr KeyCode KP2 = SDL_SCANCODE_KP_2;
        constexpr KeyCode KP3 = SDL_SCANCODE_KP_3;
        constexpr KeyCode KP4 = SDL_SCANCODE_KP_4;
        constexpr KeyCode KP5 = SDL_SCANCODE_KP_5;
        constexpr KeyCode KP6 = SDL_SCANCODE_KP_6;
        constexpr KeyCode KP7 = SDL_SCANCODE_KP_7;
        constexpr KeyCode KP8 = SDL_SCANCODE_KP_8;
        constexpr KeyCode KP9 = SDL_SCANCODE_KP_9;
        constexpr KeyCode KPDecimal = SDL_SCANCODE_KP_DECIMAL;
        constexpr KeyCode KPDivide = SDL_SCANCODE_KP_DIVIDE;
        constexpr KeyCode KPMultiply = SDL_SCANCODE_KP_MULTIPLY;
        constexpr KeyCode KPSubtract = SDL_SCANCODE_KP_MINUS;
        constexpr KeyCode KPAdd = SDL_SCANCODE_KP_PLUS;
        constexpr KeyCode KPEnter = SDL_SCANCODE_KP_ENTER;
        constexpr KeyCode KPEqual = SDL_SCANCODE_KP_EQUALS;
    } // namespace Key
} // namespace axiom
