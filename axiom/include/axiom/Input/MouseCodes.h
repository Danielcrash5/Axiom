/**
 * @file axiom/include/axiom/Input/MouseCodes.h
 * @brief Declares the public interface for the mouse codes.
 */

#pragma once

#include <SDL3/SDL.h>

namespace axiom {
    /**
     * @brief Provides the mouse code type alias.
     */
    using MouseCode = int;

    namespace mouse {
        /** @brief Maps left to its input code. */
        constexpr MouseCode left = SDL_BUTTON_LEFT;
        /** @brief Maps right to its input code. */
        constexpr MouseCode right = SDL_BUTTON_RIGHT;
        /** @brief Maps middle to its input code. */
        constexpr MouseCode middle = SDL_BUTTON_MIDDLE;
        /** @brief Maps button4 to its input code. */
        constexpr MouseCode button4 = SDL_BUTTON_X1;
        /** @brief Maps button5 to its input code. */
        constexpr MouseCode button5 = SDL_BUTTON_X2;
        /** @brief Maps button6 to its input code. */
        constexpr MouseCode button6 = 6;
        /** @brief Maps button7 to its input code. */
        constexpr MouseCode button7 = 7;
        /** @brief Maps button8 to its input code. */
        constexpr MouseCode button8 = 8;
    } // namespace Mouse
} // namespace axiom
