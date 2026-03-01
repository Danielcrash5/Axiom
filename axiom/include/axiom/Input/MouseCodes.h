#pragma once

#include <GLFW/glfw3.h>

namespace axiom {

	using MouseCode = int;

	namespace Mouse {
		constexpr MouseCode ButtonLeft = GLFW_MOUSE_BUTTON_LEFT;
		constexpr MouseCode ButtonRight = GLFW_MOUSE_BUTTON_RIGHT;
		constexpr MouseCode ButtonMiddle = GLFW_MOUSE_BUTTON_MIDDLE;

		constexpr MouseCode Button4 = GLFW_MOUSE_BUTTON_4;
		constexpr MouseCode Button5 = GLFW_MOUSE_BUTTON_5;
		constexpr MouseCode Button6 = GLFW_MOUSE_BUTTON_6;
		constexpr MouseCode Button7 = GLFW_MOUSE_BUTTON_7;
		constexpr MouseCode Button8 = GLFW_MOUSE_BUTTON_8;

		constexpr MouseCode ButtonLast = GLFW_MOUSE_BUTTON_LAST;

		// TODO: Add more mouse buttons if necessary
	}

}