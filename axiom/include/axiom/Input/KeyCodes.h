/**
 * @file axiom/include/axiom/Input/KeyCodes.h
 * @brief Declares the public interface for the key codes.
 */

#pragma once

#include <SDL3/SDL.h>

namespace axiom {
    /**
     * @brief Provides the key code type alias.
     */
    using KeyCode = int;

    namespace key {
        /** @brief Maps a to its input code. */
        constexpr KeyCode a = SDL_SCANCODE_A;
        /** @brief Maps b to its input code. */
        constexpr KeyCode b = SDL_SCANCODE_B;
        /** @brief Maps c to its input code. */
        constexpr KeyCode c = SDL_SCANCODE_C;
        /** @brief Maps d to its input code. */
        constexpr KeyCode d = SDL_SCANCODE_D;
        /** @brief Maps e to its input code. */
        constexpr KeyCode e = SDL_SCANCODE_E;
        /** @brief Maps f to its input code. */
        constexpr KeyCode f = SDL_SCANCODE_F;
        /** @brief Maps g to its input code. */
        constexpr KeyCode g = SDL_SCANCODE_G;
        /** @brief Maps h to its input code. */
        constexpr KeyCode h = SDL_SCANCODE_H;
        /** @brief Maps i to its input code. */
        constexpr KeyCode i = SDL_SCANCODE_I;
        /** @brief Maps j to its input code. */
        constexpr KeyCode j = SDL_SCANCODE_J;
        /** @brief Maps k to its input code. */
        constexpr KeyCode k = SDL_SCANCODE_K;
        /** @brief Maps l to its input code. */
        constexpr KeyCode l = SDL_SCANCODE_L;
        /** @brief Maps m to its input code. */
        constexpr KeyCode m = SDL_SCANCODE_M;
        /** @brief Maps n to its input code. */
        constexpr KeyCode n = SDL_SCANCODE_N;
        /** @brief Maps o to its input code. */
        constexpr KeyCode o = SDL_SCANCODE_O;
        /** @brief Maps p to its input code. */
        constexpr KeyCode p = SDL_SCANCODE_P;
        /** @brief Maps q to its input code. */
        constexpr KeyCode q = SDL_SCANCODE_Q;
        /** @brief Maps r to its input code. */
        constexpr KeyCode r = SDL_SCANCODE_R;
        /** @brief Maps s to its input code. */
        constexpr KeyCode s = SDL_SCANCODE_S;
        /** @brief Maps t to its input code. */
        constexpr KeyCode t = SDL_SCANCODE_T;
        /** @brief Maps u to its input code. */
        constexpr KeyCode u = SDL_SCANCODE_U;
        /** @brief Maps v to its input code. */
        constexpr KeyCode v = SDL_SCANCODE_V;
        /** @brief Maps w to its input code. */
        constexpr KeyCode w = SDL_SCANCODE_W;
        /** @brief Maps x to its input code. */
        constexpr KeyCode x = SDL_SCANCODE_X;
        /** @brief Maps y to its input code. */
        constexpr KeyCode y = SDL_SCANCODE_Y;
        /** @brief Maps z to its input code. */
        constexpr KeyCode z = SDL_SCANCODE_Z;

        /** @brief Maps num0 to its input code. */
        constexpr KeyCode num0 = SDL_SCANCODE_0;
        /** @brief Maps num1 to its input code. */
        constexpr KeyCode num1 = SDL_SCANCODE_1;
        /** @brief Maps num2 to its input code. */
        constexpr KeyCode num2 = SDL_SCANCODE_2;
        /** @brief Maps num3 to its input code. */
        constexpr KeyCode num3 = SDL_SCANCODE_3;
        /** @brief Maps num4 to its input code. */
        constexpr KeyCode num4 = SDL_SCANCODE_4;
        /** @brief Maps num5 to its input code. */
        constexpr KeyCode num5 = SDL_SCANCODE_5;
        /** @brief Maps num6 to its input code. */
        constexpr KeyCode num6 = SDL_SCANCODE_6;
        /** @brief Maps num7 to its input code. */
        constexpr KeyCode num7 = SDL_SCANCODE_7;
        /** @brief Maps num8 to its input code. */
        constexpr KeyCode num8 = SDL_SCANCODE_8;
        /** @brief Maps num9 to its input code. */
        constexpr KeyCode num9 = SDL_SCANCODE_9;

        /** @brief Maps f1 to its input code. */
        constexpr KeyCode f1 = SDL_SCANCODE_F1;
        /** @brief Maps f2 to its input code. */
        constexpr KeyCode f2 = SDL_SCANCODE_F2;
        /** @brief Maps f3 to its input code. */
        constexpr KeyCode f3 = SDL_SCANCODE_F3;
        /** @brief Maps f4 to its input code. */
        constexpr KeyCode f4 = SDL_SCANCODE_F4;
        /** @brief Maps f5 to its input code. */
        constexpr KeyCode f5 = SDL_SCANCODE_F5;
        /** @brief Maps f6 to its input code. */
        constexpr KeyCode f6 = SDL_SCANCODE_F6;
        /** @brief Maps f7 to its input code. */
        constexpr KeyCode f7 = SDL_SCANCODE_F7;
        /** @brief Maps f8 to its input code. */
        constexpr KeyCode f8 = SDL_SCANCODE_F8;
        /** @brief Maps f9 to its input code. */
        constexpr KeyCode f9 = SDL_SCANCODE_F9;
        /** @brief Maps f10 to its input code. */
        constexpr KeyCode f10 = SDL_SCANCODE_F10;
        /** @brief Maps f11 to its input code. */
        constexpr KeyCode f11 = SDL_SCANCODE_F11;
        /** @brief Maps f12 to its input code. */
        constexpr KeyCode f12 = SDL_SCANCODE_F12;
        /** @brief Maps f13 to its input code. */
        constexpr KeyCode f13 = SDL_SCANCODE_F13;
        /** @brief Maps f14 to its input code. */
        constexpr KeyCode f14 = SDL_SCANCODE_F14;
        /** @brief Maps f15 to its input code. */
        constexpr KeyCode f15 = SDL_SCANCODE_F15;
        /** @brief Maps f16 to its input code. */
        constexpr KeyCode f16 = SDL_SCANCODE_F16;
        /** @brief Maps f17 to its input code. */
        constexpr KeyCode f17 = SDL_SCANCODE_F17;
        /** @brief Maps f18 to its input code. */
        constexpr KeyCode f18 = SDL_SCANCODE_F18;
        /** @brief Maps f19 to its input code. */
        constexpr KeyCode f19 = SDL_SCANCODE_F19;
        /** @brief Maps f20 to its input code. */
        constexpr KeyCode f20 = SDL_SCANCODE_F20;
        /** @brief Maps f21 to its input code. */
        constexpr KeyCode f21 = SDL_SCANCODE_F21;
        /** @brief Maps f22 to its input code. */
        constexpr KeyCode f22 = SDL_SCANCODE_F22;
        /** @brief Maps f23 to its input code. */
        constexpr KeyCode f23 = SDL_SCANCODE_F23;
        /** @brief Maps f24 to its input code. */
        constexpr KeyCode f24 = SDL_SCANCODE_F24;

        /** @brief Maps space to its input code. */
        constexpr KeyCode space = SDL_SCANCODE_SPACE;
        /** @brief Maps escape to its input code. */
        constexpr KeyCode escape = SDL_SCANCODE_ESCAPE;
        /** @brief Maps enter to its input code. */
        constexpr KeyCode enter = SDL_SCANCODE_RETURN;
        /** @brief Maps tab to its input code. */
        constexpr KeyCode tab = SDL_SCANCODE_TAB;
        /** @brief Maps backspace to its input code. */
        constexpr KeyCode backspace = SDL_SCANCODE_BACKSPACE;
        /** @brief Maps insert to its input code. */
        constexpr KeyCode insert = SDL_SCANCODE_INSERT;
        /** @brief Maps delete key to its input code. */
        constexpr KeyCode deleteKey = SDL_SCANCODE_DELETE;
        /** @brief Maps right to its input code. */
        constexpr KeyCode right = SDL_SCANCODE_RIGHT;
        /** @brief Maps left to its input code. */
        constexpr KeyCode left = SDL_SCANCODE_LEFT;
        /** @brief Maps down to its input code. */
        constexpr KeyCode down = SDL_SCANCODE_DOWN;
        /** @brief Maps up to its input code. */
        constexpr KeyCode up = SDL_SCANCODE_UP;
        /** @brief Maps page up to its input code. */
        constexpr KeyCode pageUp = SDL_SCANCODE_PAGEUP;
        /** @brief Maps page down to its input code. */
        constexpr KeyCode pageDown = SDL_SCANCODE_PAGEDOWN;
        /** @brief Maps home to its input code. */
        constexpr KeyCode home = SDL_SCANCODE_HOME;
        /** @brief Maps end to its input code. */
        constexpr KeyCode end = SDL_SCANCODE_END;
        /** @brief Maps caps lock to its input code. */
        constexpr KeyCode capsLock = SDL_SCANCODE_CAPSLOCK;
        /** @brief Maps scroll lock to its input code. */
        constexpr KeyCode scrollLock = SDL_SCANCODE_SCROLLLOCK;
        /** @brief Maps num lock to its input code. */
        constexpr KeyCode numLock = SDL_SCANCODE_NUMLOCKCLEAR;
        /** @brief Maps print screen to its input code. */
        constexpr KeyCode printScreen = SDL_SCANCODE_PRINTSCREEN;
        /** @brief Maps pause to its input code. */
        constexpr KeyCode pause = SDL_SCANCODE_PAUSE;
        /** @brief Maps menu to its input code. */
        constexpr KeyCode menu = SDL_SCANCODE_MENU;

        /** @brief Maps left shift to its input code. */
        constexpr KeyCode leftShift = SDL_SCANCODE_LSHIFT;
        /** @brief Maps left control to its input code. */
        constexpr KeyCode leftControl = SDL_SCANCODE_LCTRL;
        /** @brief Maps left alt to its input code. */
        constexpr KeyCode leftAlt = SDL_SCANCODE_LALT;
        /** @brief Maps left super to its input code. */
        constexpr KeyCode leftSuper = SDL_SCANCODE_LGUI;
        /** @brief Maps right shift to its input code. */
        constexpr KeyCode rightShift = SDL_SCANCODE_RSHIFT;
        /** @brief Maps right control to its input code. */
        constexpr KeyCode rightControl = SDL_SCANCODE_RCTRL;
        /** @brief Maps right alt to its input code. */
        constexpr KeyCode rightAlt = SDL_SCANCODE_RALT;
        /** @brief Maps right super to its input code. */
        constexpr KeyCode rightSuper = SDL_SCANCODE_RGUI;

        /** @brief Maps k p0 to its input code. */
        constexpr KeyCode kP0 = SDL_SCANCODE_KP_0;
        /** @brief Maps k p1 to its input code. */
        constexpr KeyCode kP1 = SDL_SCANCODE_KP_1;
        /** @brief Maps k p2 to its input code. */
        constexpr KeyCode kP2 = SDL_SCANCODE_KP_2;
        /** @brief Maps k p3 to its input code. */
        constexpr KeyCode kP3 = SDL_SCANCODE_KP_3;
        /** @brief Maps k p4 to its input code. */
        constexpr KeyCode kP4 = SDL_SCANCODE_KP_4;
        /** @brief Maps k p5 to its input code. */
        constexpr KeyCode kP5 = SDL_SCANCODE_KP_5;
        /** @brief Maps k p6 to its input code. */
        constexpr KeyCode kP6 = SDL_SCANCODE_KP_6;
        /** @brief Maps k p7 to its input code. */
        constexpr KeyCode kP7 = SDL_SCANCODE_KP_7;
        /** @brief Maps k p8 to its input code. */
        constexpr KeyCode kP8 = SDL_SCANCODE_KP_8;
        /** @brief Maps k p9 to its input code. */
        constexpr KeyCode kP9 = SDL_SCANCODE_KP_9;
        /** @brief Maps kp decimal to its input code. */
        constexpr KeyCode kpDecimal = SDL_SCANCODE_KP_DECIMAL;
        /** @brief Maps kp divide to its input code. */
        constexpr KeyCode kpDivide = SDL_SCANCODE_KP_DIVIDE;
        /** @brief Maps kp multiply to its input code. */
        constexpr KeyCode kpMultiply = SDL_SCANCODE_KP_MULTIPLY;
        /** @brief Maps kp subtract to its input code. */
        constexpr KeyCode kpSubtract = SDL_SCANCODE_KP_MINUS;
        /** @brief Maps kp add to its input code. */
        constexpr KeyCode kpAdd = SDL_SCANCODE_KP_PLUS;
        /** @brief Maps kp enter to its input code. */
        constexpr KeyCode kpEnter = SDL_SCANCODE_KP_ENTER;
        /** @brief Maps kp equal to its input code. */
        constexpr KeyCode kpEqual = SDL_SCANCODE_KP_EQUALS;
    } // namespace Key
} // namespace axiom
