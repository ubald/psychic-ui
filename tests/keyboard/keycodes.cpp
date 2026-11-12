#include <catch2/catch_test_macros.hpp>
#include <psychic-ui/applications/SDL3SystemWindow.hpp>

using namespace psychic_ui;

TEST_CASE("SDL3 keycodes map to Psychic UI keys", "[keyboard][sdl3]") {
    CHECK(SDL3SystemWindow::mapKey(SDLK_A) == Key::A);
    CHECK(SDL3SystemWindow::mapKey(SDLK_Z) == Key::Z);
    CHECK(SDL3SystemWindow::mapKey(SDLK_0) == Key::ZERO);
    CHECK(SDL3SystemWindow::mapKey(SDLK_9) == Key::NINE);
    CHECK(SDL3SystemWindow::mapKey(SDLK_RETURN) == Key::ENTER);
    CHECK(SDL3SystemWindow::mapKey(SDLK_BACKSPACE) == Key::BACKSPACE);
    CHECK(SDL3SystemWindow::mapKey(SDLK_DELETE) == Key::DELETE);
    CHECK(SDL3SystemWindow::mapKey(SDLK_LEFT) == Key::LEFT);
    CHECK(SDL3SystemWindow::mapKey(SDLK_HOME) == Key::HOME);
    CHECK(SDL3SystemWindow::mapKey(SDLK_APOSTROPHE) == Key::APOSTROPHE);
    CHECK(SDL3SystemWindow::mapKey(SDLK_GRAVE) == Key::GRAVE_ACCENT);
    CHECK(SDL3SystemWindow::mapKey(SDLK_F12) == Key::F12);
    CHECK(SDL3SystemWindow::mapKey(SDLK_KP_ENTER) == Key::KP_ENTER);
    CHECK(SDL3SystemWindow::mapKey(SDLK_LGUI) == Key::LEFT_SUPER);
    CHECK(SDL3SystemWindow::mapKey(SDLK_APPLICATION) == Key::MENU);
    CHECK(SDL3SystemWindow::mapKey(SDLK_UNKNOWN) == Key::UNKNOWN);
}

TEST_CASE("SDL3 modifiers map to Psychic UI modifiers", "[keyboard][sdl3]") {
    const Mod none = SDL3SystemWindow::mapMods(SDL_KMOD_NONE);
    CHECK_FALSE((none.shift || none.ctrl || none.alt || none.super));

    // Either side counts
    const Mod mod = SDL3SystemWindow::mapMods(SDL_KMOD_LSHIFT | SDL_KMOD_RCTRL | SDL_KMOD_LALT | SDL_KMOD_RGUI);
    CHECK(mod.shift);
    CHECK(mod.ctrl);
    CHECK(mod.alt);
    CHECK(mod.super);

    // Lock keys aren't modifiers
    const Mod locks = SDL3SystemWindow::mapMods(SDL_KMOD_CAPS | SDL_KMOD_NUM);
    CHECK_FALSE((locks.shift || locks.ctrl || locks.alt || locks.super));
}
