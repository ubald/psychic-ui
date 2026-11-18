#pragma once

#include <cstdint>
#include "include/core/SkColor.h"
//#include <rxcpp/rx.hpp>

// Configuration
#define DEBUG_LAYOUT

// Define command key for windows/mac/linux
#ifdef __APPLE__
#define SYSTEM_COMMAND_MOD GLFW_MOD_SUPER
#else
#define SYSTEM_COMMAND_MOD GLFW_MOD_CONTROL
#endif

namespace psychic_ui {

    enum class MouseButton : std::uint8_t {
        NONE = 0,
        LEFT = 1,
        MIDDLE = 2,
        RIGHT = 4
    };

    /**
     * What the user is pointing with, as the window last saw it (`Window::inputMode()`): a mouse or trackpad,
     * a finger on a touch screen, or a pen. Touch makes the style manager switch to touch density.
     */
    enum class InputMode : std::uint8_t {
        Pointer,
        Touch,
        Pen
    };

    struct Mod {
        bool shift{false};
        bool ctrl{false};
        bool alt{false};
        bool super{false};
    };

    /**
     * What every mouse signal passes (`mousePressed`, `clicked`, `mouseScrolled`, `contextMenuRequested`...), also
     * made of touches (`source` is `Touch`), pens and keys (a keyboard click or context menu, `clickCount` 0).
     * Each div gets it in its own coordinates: the window fills it in window coordinates and every div on the way
     * moves `x` and `y` into its own.
     */
    struct MouseEvent {
        /**
         * Where, in the receiving div's content coordinates (those its children's `x()` and `y()` are in): from its
         * top-left corner, plus how far it is scrolled
         */
        float       x{0};
        float       y{0};
        /**
         * Where, in window coordinates, whichever div receives it
         */
        float       windowX{0};
        float       windowY{0};
        /**
         * The button that changed (press, release, button change, click, context menu); NONE for moves and scrolls
         */
        MouseButton button{MouseButton::NONE};
        /**
         * Whether `button` went down, for `mouseButtonChanged` (true for presses, false for releases and clicks)
         */
        bool        down{false};
        /**
         * Every button held (MouseButton bits), after this change
         */
        unsigned    buttons{0};
        Mod         modifiers{};
        /**
         * Presses in a row of this button at this spot: 1, 2 for a double click, 3... Kept from the press for its
         * release and click; 0 when no press made it (moves, scrolls, keyboard clicks and context menus)
         */
        unsigned    clickCount{0};
        /**
         * What made it: the mouse, a finger (the window's touch gestures) or a pen
         */
        InputMode   source{InputMode::Pointer};
        /**
         * Made by a key in place of the mouse (`Div::mouseEventAtCenter`: a click by Enter or Space, a context menu by
         * the Menu key or Shift+F10), so that what it opens can start at its first item, as from the keyboard
         */
        bool        fromKeyboard{false};
        /**
         * How far the wheel turned, for `mouseScrolled`, in steps as SDL and GLFW report them (a positive `scrollY` is
         * away from the user, scrolling up)
         */
        double      scrollX{0};
        double      scrollY{0};
    };

    /**
     * Since we support either SDL2 or GLFW, we have to map keyboard keys
     * to something we can use (they don't use the same values)
     *
     * Here we're using the set of keys from GLFW since it names less keys than SDL
     */
    enum class Key : std::uint8_t {
        UNKNOWN,
        SPACE,
        APOSTROPHE,
        COMMA,
        MINUS,
        PERIOD,
        SLASH,
        ZERO,
        ONE,
        TWO,
        THREE,
        FOUR,
        FIVE,
        SIX,
        SEVEN,
        EIGHT,
        NINE,
        SEMICOLON,
        EQUAL,
        A,
        B,
        C,
        D,
        E,
        F,
        G,
        H,
        I,
        J,
        K,
        L,
        M,
        N,
        O,
        P,
        Q,
        R,
        S,
        T,
        U,
        V,
        W,
        X,
        Y,
        Z,
        LEFT_BRACKET,
        BACKSLASH,
        RIGHT_BRACKET,
        GRAVE_ACCENT,
        WORLD_1,
        WORLD_2,
        ESCAPE,
        ENTER,
        TAB,
        BACKSPACE,
        INSERT,
        DELETE,
        RIGHT,
        LEFT,
        DOWN,
        UP,
        PAGE_UP,
        PAGE_DOWN,
        HOME,
        END,
        CAPS_LOCK,
        SCROLL_LOCK,
        NUM_LOCK,
        PRINT_SCREEN,
        PAUSE,
        F1,
        F2,
        F3,
        F4,
        F5,
        F6,
        F7,
        F8,
        F9,
        F10,
        F11,
        F12,
        F13,
        F14,
        F15,
        F16,
        F17,
        F18,
        F19,
        F20,
        F21,
        F22,
        F23,
        F24,
        F25,
        KP_0,
        KP_1,
        KP_2,
        KP_3,
        KP_4,
        KP_5,
        KP_6,
        KP_7,
        KP_8,
        KP_9,
        KP_DECIMAL,
        KP_DIVIDE,
        KP_MULTIPLY,
        KP_SUBTRACT,
        KP_ADD,
        KP_ENTER,
        KP_EQUAL,
        LEFT_SHIFT,
        LEFT_CONTROL,
        LEFT_ALT,
        LEFT_SUPER,
        RIGHT_SHIFT,
        RIGHT_CONTROL,
        RIGHT_ALT,
        RIGHT_SUPER,
        MENU,
    };

}
