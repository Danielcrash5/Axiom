#pragma once

#include <SDL3/SDL.h>

namespace axiom {
    using MouseCode = int;

    namespace Mouse {
        constexpr MouseCode Left = SDL_BUTTON_LEFT;
        constexpr MouseCode Right = SDL_BUTTON_RIGHT;
        constexpr MouseCode Middle = SDL_BUTTON_MIDDLE;
        constexpr MouseCode Button4 = SDL_BUTTON_X1;
        constexpr MouseCode Button5 = SDL_BUTTON_X2;
        constexpr MouseCode Button6 = 6;
        constexpr MouseCode Button7 = 7;
        constexpr MouseCode Button8 = 8;
    }
}
