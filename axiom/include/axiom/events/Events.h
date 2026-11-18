/**
 * @file axiom/include/axiom/events/Events.h
 * @brief Declares the public interface for the events.
 */

#pragma once

namespace axiom {
    //------------------------------------------------
    //------------- Window Events --------------------
    //------------------------------------------------

    /** @brief Carries the data for the window close event event. */
    struct WindowCloseEvent {
        /** @brief Stores the closed. */
        bool closed = true;
    };

    /** @brief Carries the data for the window resize event event. */
    struct WindowResizeEvent {
        /** @brief Stores the width. */
        uint32_t width;
        /** @brief Stores the height. */
        uint32_t height;
    };

    //------------------------------------------------
    //------------- Input Events ---------------------
    //------------------------------------------------

    /** @brief Carries the data for the key event event. */
    struct KeyEvent {
        /** @brief Stores the key. */
        int key;
        /** @brief Stores the scancode. */
        int scancode;
        /** @brief Stores the action. */
        int action;
        /** @brief Stores the mods. */
        int mods;
        /** @brief Stores the repeat. */
        int repeat;
    };

    /** @brief Carries the data for the mouse button event event. */
    struct MouseButtonEvent {
        /** @brief Stores the button. */
        int button;
        /** @brief Stores the action. */
        int action;
        /** @brief Stores the mods. */
        int mods;
    };

    /** @brief Carries the data for the mouse move event event. */
    struct MouseMoveEvent {
        /** @brief Stores the x. */
        double x;
        /** @brief Stores the y. */
        double y;
    };

    /** @brief Carries the data for the mouse scroll event event. */
    struct MouseScrollEvent {
        /** @brief Stores the x offset. */
        double xOffset;
        /** @brief Stores the y offset. */
        double yOffset;
    };

    /** @brief Carries the data for the char event event. */
    struct CharEvent {
        /** @brief Stores the codepoint. */
        unsigned int codepoint;
    };

    /** @brief Carries the data for the char mods event event. */
    struct CharModsEvent {
        /** @brief Stores the codepoint. */
        unsigned int codepoint;
        /** @brief Stores the mods. */
        int mods;
    };

    /** @brief Carries the data for the file drop event event. */
    struct FileDropEvent {
        /** @brief Stores the count. */
        int count;
        /** @brief Stores the paths. */
        const char **paths;
    };
} // namespace axiom
