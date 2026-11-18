// Known bugs from review-2026-11.md, tagged [!shouldfail]: remove the tag in the commit that fixes one.
#include <chrono>
#include <catch2/catch_test_macros.hpp>
#include <psychic-ui/components/Button.hpp>
#include <psychic-ui/components/Dialog.hpp>
#include <psychic-ui/components/Menu.hpp>
#include "../backends/fake_system_window.hpp"

using namespace psychic_ui;
using namespace psychic_ui_tests;
using namespace std::chrono_literals;

namespace {
    struct Modal {
        std::shared_ptr<TestWindow> window{std::make_shared<TestWindow>()};
        FakeSystemWindow            sys{window, 800, 600};

        Button &button(const std::string &label, float left, float top) {
            return window->appContainer().add<Button>(label).position(Position::Absolute).left(left).top(top).width(100).height(40);
        }

        void click(float x, float y) {
            window->handleMouseButtonChange({.x = x, .y = y, .button = MouseButton::LEFT, .down = true});
            window->handleMouseButtonChange({.x = x, .y = y, .button = MouseButton::LEFT, .down = false});
        }
    };
}

// C9: the hover path isn't cut at a modal popup, so what is under it stays hovered and gets the moves
TEST_CASE("C9: under a modal dialog, the div under the mouse loses its hover and its moves", "[popup][modal][mouse][!shouldfail]") {
    Modal f;
    auto &button = f.button("Under", 10, 10);
    int   moves  = 0;
    button.onMouseMove([&] { ++moves; });
    f.sys.layout();
    f.window->handleMouseMove({.x = 50, .y = 30}, false);
    REQUIRE(button.mouseOver());
    f.window->overlay().open<Dialog>("Modal");
    f.sys.layout();
    moves = 0;
    f.window->handleMouseMove({.x = 52, .y = 30}, false);
    f.window->handleMouseMove({.x = 54, .y = 31}, false);
    CHECK_FALSE(button.mouseOver());
    CHECK(moves == 0);
}

TEST_CASE("No tooltip shows for a div under a modal dialog", "[popup][modal][tooltip]") {
    Modal f;
    f.button("Under", 10, 10).tooltip("Tip");
    f.sys.layout();
    f.window->overlay().open<Dialog>("Modal");
    f.sys.layout();
    f.window->handleMouseMove({.x = 50, .y = 30}, false);
    f.window->updatePopups(std::chrono::nanoseconds(1s));
    f.window->updatePopups(std::chrono::nanoseconds(2s));
    f.sys.layout();
    CHECK(f.window->overlay().children().size() == 1); // the dialog only
}

// C10: a touch cancel never clears _pressSwallowed, set when the cancelled press closed a modal popup
TEST_CASE("C10: after a touch cancel whose press closed a menu, the next click goes through", "[popup][touch][!shouldfail]") {
    Modal f;
    auto &button = f.button("Go", 400, 400);
    int   clicks = 0;
    button.onClick([&] { ++clicks; });
    f.sys.layout();
    f.window->openMenu({Menu::item("One", []() {})}, 10, 10);
    f.sys.layout();
    // A finger lands outside the menu, which closes it, then drags and is cancelled
    f.window->handleTouchPress(300, 300, 1000ms);
    f.window->handleTouchMove(300, 330, 1010ms);
    f.window->handleTouchCancel(1020ms);
    f.sys.layout();
    f.window->handleInputModeChange(InputMode::Pointer);
    f.sys.layout();
    f.window->handleMouseMove({.x = 450, .y = 420}, false);
    f.click(450, 420);
    CHECK(clicks == 1);
}

// C15: a popup gives the focus back to what had it, even when that has since been disabled or hidden
TEST_CASE("C15: a menu closing doesn't give the focus back to a trigger disabled meanwhile", "[popup][menu][focus][!shouldfail]") {
    Modal f;
    auto &button = f.button("Menu", 100, 100);
    f.sys.layout();
    f.window->requestFocus(&button, true);
    Menu &menu = f.window->openMenu({Menu::item("One", []() {})}, button);
    f.sys.layout();
    f.window->requestFocus(&menu, true);
    button.enabled(false);
    button.visible(false);
    f.sys.layout();
    menu.close();
    CHECK(f.window->focusedDiv() != &button);
}
