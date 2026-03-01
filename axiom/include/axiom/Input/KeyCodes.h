#pragma once
#include <GLFW/glfw3.h>

namespace axiom {
    using KeyCode = int;

    namespace Key {
        // Letters
        constexpr KeyCode A = GLFW_KEY_A;
        constexpr KeyCode B = GLFW_KEY_B;
        constexpr KeyCode C = GLFW_KEY_C;
        constexpr KeyCode D = GLFW_KEY_D;
        constexpr KeyCode E = GLFW_KEY_E;
        constexpr KeyCode F = GLFW_KEY_F;
        constexpr KeyCode G = GLFW_KEY_G;
        constexpr KeyCode H = GLFW_KEY_H;
        constexpr KeyCode I = GLFW_KEY_I;
        constexpr KeyCode J = GLFW_KEY_J;
        constexpr KeyCode K = GLFW_KEY_K;
        constexpr KeyCode L = GLFW_KEY_L;
        constexpr KeyCode M = GLFW_KEY_M;
        constexpr KeyCode N = GLFW_KEY_N;
        constexpr KeyCode O = GLFW_KEY_O;
        constexpr KeyCode P = GLFW_KEY_P;
        constexpr KeyCode Q = GLFW_KEY_Q;
        constexpr KeyCode R = GLFW_KEY_R;
        constexpr KeyCode S = GLFW_KEY_S;
        constexpr KeyCode T = GLFW_KEY_T;
        constexpr KeyCode U = GLFW_KEY_U;
        constexpr KeyCode V = GLFW_KEY_V;
        constexpr KeyCode W = GLFW_KEY_W;
        constexpr KeyCode X = GLFW_KEY_X;
        constexpr KeyCode Y = GLFW_KEY_Y;
        constexpr KeyCode Z = GLFW_KEY_Z;

        // Numbers
        constexpr KeyCode Num0 = GLFW_KEY_0;
        constexpr KeyCode Num1 = GLFW_KEY_1;
        constexpr KeyCode Num2 = GLFW_KEY_2;
        constexpr KeyCode Num3 = GLFW_KEY_3;
        constexpr KeyCode Num4 = GLFW_KEY_4;
        constexpr KeyCode Num5 = GLFW_KEY_5;
        constexpr KeyCode Num6 = GLFW_KEY_6;
        constexpr KeyCode Num7 = GLFW_KEY_7;
        constexpr KeyCode Num8 = GLFW_KEY_8;
        constexpr KeyCode Num9 = GLFW_KEY_9;

        // Function keys
        constexpr KeyCode F1 = GLFW_KEY_F1;
        constexpr KeyCode F2 = GLFW_KEY_F2;
        constexpr KeyCode F3 = GLFW_KEY_F3;
        constexpr KeyCode F4 = GLFW_KEY_F4;
        constexpr KeyCode F5 = GLFW_KEY_F5;
        constexpr KeyCode F6 = GLFW_KEY_F6;
        constexpr KeyCode F7 = GLFW_KEY_F7;
        constexpr KeyCode F8 = GLFW_KEY_F8;
        constexpr KeyCode F9 = GLFW_KEY_F9;
        constexpr KeyCode F10 = GLFW_KEY_F10;
        constexpr KeyCode F11 = GLFW_KEY_F11;
        constexpr KeyCode F12 = GLFW_KEY_F12;
        constexpr KeyCode F13 = GLFW_KEY_F13;
        constexpr KeyCode F14 = GLFW_KEY_F14;
        constexpr KeyCode F15 = GLFW_KEY_F15;
        constexpr KeyCode F16 = GLFW_KEY_F16;
        constexpr KeyCode F17 = GLFW_KEY_F17;
        constexpr KeyCode F18 = GLFW_KEY_F18;
        constexpr KeyCode F19 = GLFW_KEY_F19;
        constexpr KeyCode F20 = GLFW_KEY_F20;
        constexpr KeyCode F21 = GLFW_KEY_F21;
        constexpr KeyCode F22 = GLFW_KEY_F22;
        constexpr KeyCode F23 = GLFW_KEY_F23;
        constexpr KeyCode F24 = GLFW_KEY_F24;

        // Special keys
		constexpr KeyCode Space = GLFW_KEY_SPACE;
        constexpr KeyCode Escape = GLFW_KEY_ESCAPE;
        constexpr KeyCode Enter = GLFW_KEY_ENTER;
        constexpr KeyCode Tab = GLFW_KEY_TAB;
        constexpr KeyCode Backspace = GLFW_KEY_BACKSPACE;
        constexpr KeyCode Insert = GLFW_KEY_INSERT;
        constexpr KeyCode Delete = GLFW_KEY_DELETE;
        constexpr KeyCode Right = GLFW_KEY_RIGHT;
        constexpr KeyCode Left = GLFW_KEY_LEFT;
        constexpr KeyCode Down = GLFW_KEY_DOWN;
        constexpr KeyCode Up = GLFW_KEY_UP;
        constexpr KeyCode PageUp = GLFW_KEY_PAGE_UP;
        constexpr KeyCode PageDown = GLFW_KEY_PAGE_DOWN;
        constexpr KeyCode Home = GLFW_KEY_HOME;
        constexpr KeyCode End = GLFW_KEY_END;
        constexpr KeyCode CapsLock = GLFW_KEY_CAPS_LOCK;
        constexpr KeyCode ScrollLock = GLFW_KEY_SCROLL_LOCK;
        constexpr KeyCode NumLock = GLFW_KEY_NUM_LOCK;
        constexpr KeyCode PrintScreen = GLFW_KEY_PRINT_SCREEN;
        constexpr KeyCode Pause = GLFW_KEY_PAUSE;
        constexpr KeyCode Menu = GLFW_KEY_MENU;

        // Modifiers
        constexpr KeyCode LeftShift = GLFW_KEY_LEFT_SHIFT;
        constexpr KeyCode LeftControl = GLFW_KEY_LEFT_CONTROL;
        constexpr KeyCode LeftAlt = GLFW_KEY_LEFT_ALT;
        constexpr KeyCode LeftSuper = GLFW_KEY_LEFT_SUPER;
        constexpr KeyCode RightShift = GLFW_KEY_RIGHT_SHIFT;
        constexpr KeyCode RightControl = GLFW_KEY_RIGHT_CONTROL;
        constexpr KeyCode RightAlt = GLFW_KEY_RIGHT_ALT;
        constexpr KeyCode RightSuper = GLFW_KEY_RIGHT_SUPER;

        // Numpad
        constexpr KeyCode KP0 = GLFW_KEY_KP_0;
        constexpr KeyCode KP1 = GLFW_KEY_KP_1;
        constexpr KeyCode KP2 = GLFW_KEY_KP_2;
        constexpr KeyCode KP3 = GLFW_KEY_KP_3;
        constexpr KeyCode KP4 = GLFW_KEY_KP_4;
        constexpr KeyCode KP5 = GLFW_KEY_KP_5;
        constexpr KeyCode KP6 = GLFW_KEY_KP_6;
        constexpr KeyCode KP7 = GLFW_KEY_KP_7;
        constexpr KeyCode KP8 = GLFW_KEY_KP_8;
        constexpr KeyCode KP9 = GLFW_KEY_KP_9;
        constexpr KeyCode KPDecimal = GLFW_KEY_KP_DECIMAL;
        constexpr KeyCode KPDivide = GLFW_KEY_KP_DIVIDE;
        constexpr KeyCode KPMultiply = GLFW_KEY_KP_MULTIPLY;
        constexpr KeyCode KPSubtract = GLFW_KEY_KP_SUBTRACT;
        constexpr KeyCode KPAdd = GLFW_KEY_KP_ADD;
        constexpr KeyCode KPEnter = GLFW_KEY_KP_ENTER;
        constexpr KeyCode KPEqual = GLFW_KEY_KP_EQUAL;
    }
}