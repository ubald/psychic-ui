#if defined(WITH_SDL2) && defined(WITH_GLFW)

#include "catch.hpp"
#include <iostream>

#include <SDL.h>
#include <GLFW/glfw3.h>

/**
 * This is not a real test,
 * its only a simple way to verify that both libraries
 * return the same keycodes
 */
TEST_CASE( "Check supported keycodes" ) {
    CHECK(SDLK_RETURN == GLFW_KEY_ENTER);
    CHECK(SDLK_RETURN2 == GLFW_KEY_ENTER);
    CHECK(SDLK_ESCAPE== GLFW_KEY_ESCAPE);
    CHECK(SDLK_BACKSPACE == GLFW_KEY_BACKSPACE);
    CHECK(SDLK_TAB == GLFW_KEY_TAB);

    CHECK(SDLK_SPACE == GLFW_KEY_SPACE);
    CHECK(SDLK_COMMA == GLFW_KEY_COMMA);
    CHECK(SDLK_COMMA == GLFW_KEY_COMMA);
    CHECK(SDLK_BACKSLASH == GLFW_KEY_BACKSLASH);
    CHECK(SDLK_LEFTBRACKET == GLFW_KEY_LEFT_BRACKET);
    CHECK(SDLK_RIGHTBRACKET == GLFW_KEY_RIGHT_BRACKET);
}

#endif
