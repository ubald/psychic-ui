// Known bugs from docs/review-2026-11.md, tagged [!shouldfail]: remove the tag in the commit that fixes one.
#include <catch2/catch_test_macros.hpp>
#include "../backends/fake_system_window.hpp"

using namespace psychic_ui;
using namespace psychic_ui_tests;

namespace {
    struct Presses {
        std::shared_ptr<TestWindow> window{std::make_shared<TestWindow>()};
        FakeSystemWindow            sys{window, 800, 600};

        void press(float x, float y, MouseButton button = MouseButton::LEFT) {
            window->handleMouseButtonChange({.x = x, .y = y, .button = button, .down = true});
        }

        void release(float x, float y, MouseButton button = MouseButton::LEFT) {
            window->handleMouseButtonChange({.x = x, .y = y, .button = button, .down = false});
        }
    };
}

// C3: Div::handleMouseRelease returns early for a hidden div, so it never hears the release of its press
TEST_CASE("C3: a div hidden mid-press hears the release and stops being pressed", "[div][events][mouse][!shouldfail]") {
    Presses f;
    auto   &row = f.window->appContainer().add<Div>().position(Position::Absolute).left(0).top(0).width(400).height(100).direction(Direction::Row);
    auto   &a   = row.add<Div>().width(100).height(100);
    auto   &b   = row.add<Div>().width(100).height(100);
    int     clicks = 0, released = 0;
    b.onClick([&] { ++clicks; });
    b.onMouseRelease([&] { ++released; });
    f.sys.layout();

    f.press(150, 50);
    b.visible(false);
    f.sys.layout();
    f.release(150, 50);
    CHECK(released == 1);

    b.visible(true);
    f.sys.layout();
    CHECK_FALSE(b.active());

    // A press on a sibling released over b doesn't click b
    f.press(50, 50);
    (void) a;
    f.release(150, 50);
    CHECK(clicks == 0);
}

// C3: the pressed state is one flag for every button, so the second button's release ends the first's press
TEST_CASE("C3: releasing a second button keeps the first button's press", "[div][events][mouse][!shouldfail]") {
    Presses f;
    auto   &a = f.window->appContainer().add<Div>().position(Position::Absolute).left(0).top(0).width(100).height(100);
    int     clicks = 0;
    a.onClick([&] { ++clicks; });
    f.sys.layout();
    f.press(50, 50);
    f.press(50, 50, MouseButton::RIGHT);
    f.release(50, 50, MouseButton::RIGHT);
    CHECK(a.active());
    f.release(50, 50);
    CHECK(clicks == 1);
}
