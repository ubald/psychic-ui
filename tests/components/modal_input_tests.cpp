// Known bugs from review-2026-11.md, tagged [!shouldfail]: remove the tag in the commit that fixes one.
#include <chrono>
#include <memory>
#include <string>
#include <catch2/catch_test_macros.hpp>
#include <psychic-ui/components/Button.hpp>
#include <psychic-ui/components/Dialog.hpp>
#include <psychic-ui/components/Menu.hpp>
#include <psychic-ui/psychic-ui.hpp>
#include <psychic-ui/style/Values.hpp>
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
    Modal fixture;
    auto &button = fixture.button("Under", 10, 10);
    int   moves  = 0;
    button.onMouseMove([&] { ++moves; });
    fixture.sys.layout();
    fixture.window->handleMouseMove({.x = 50, .y = 30}, false);
    REQUIRE(button.mouseOver());
    fixture.window->overlay().open<Dialog>("Modal");
    fixture.sys.layout();
    moves = 0;
    fixture.window->handleMouseMove({.x = 52, .y = 30}, false);
    fixture.window->handleMouseMove({.x = 54, .y = 31}, false);
    CHECK_FALSE(button.mouseOver());
    CHECK(moves == 0);
}

TEST_CASE("No tooltip shows for a div under a modal dialog", "[popup][modal][tooltip]") {
    Modal fixture;
    fixture.button("Under", 10, 10).tooltip("Tip");
    fixture.sys.layout();
    fixture.window->overlay().open<Dialog>("Modal");
    fixture.sys.layout();
    fixture.window->handleMouseMove({.x = 50, .y = 30}, false);
    fixture.window->updatePopups(std::chrono::nanoseconds(1s));
    fixture.window->updatePopups(std::chrono::nanoseconds(2s));
    fixture.sys.layout();
    CHECK(fixture.window->overlay().children().size() == 1); // the dialog only
}

// C10: a touch cancel never clears _pressSwallowed, set when the cancelled press closed a modal popup
TEST_CASE("C10: after a touch cancel whose press closed a menu, the next click goes through", "[popup][touch][!shouldfail]") {
    Modal fixture;
    auto &button = fixture.button("Go", 400, 400);
    int   clicks = 0;
    button.onClick([&] { ++clicks; });
    fixture.sys.layout();
    fixture.window->openMenu({Menu::item("One", []() {})}, 10, 10);
    fixture.sys.layout();
    // A finger lands outside the menu, which closes it, then drags and is cancelled
    fixture.window->handleTouchPress(300, 300, 1000ms);
    fixture.window->handleTouchMove(300, 330, 1010ms);
    fixture.window->handleTouchCancel(1020ms);
    fixture.sys.layout();
    fixture.window->handleInputModeChange(InputMode::Pointer);
    fixture.sys.layout();
    fixture.window->handleMouseMove({.x = 450, .y = 420}, false);
    fixture.click(450, 420);
    CHECK(clicks == 1);
}

// C15: a popup gives the focus back to what had it, unless that has since been disabled or hidden
TEST_CASE("C15: a menu closing doesn't give the focus back to a trigger disabled meanwhile", "[popup][menu][focus]") {
    Modal fixture;
    auto &button = fixture.button("Menu", 100, 100);
    fixture.sys.layout();
    fixture.window->requestFocus(&button, true);
    Menu &menu = fixture.window->openMenu({Menu::item("One", []() {})}, button);
    fixture.sys.layout();
    fixture.window->requestFocus(&menu, true);
    button.enabled(false);
    button.visible(false);
    fixture.sys.layout();
    menu.close();
    CHECK(fixture.window->focusedDiv() != &button);
}
