// Known bugs from review-2026-11.md, tagged [!shouldfail]: remove the tag in the commit that fixes one.
#include <memory>
#include <catch2/catch_test_macros.hpp>
#include "../backends/fake_system_window.hpp"
#include <psychic-ui/Div.hpp>
#include <psychic-ui/style/Values.hpp>

using namespace psychic_ui;
using namespace psychic_ui_tests;

namespace {
    struct Scrolling {
        std::shared_ptr<TestWindow> window{std::make_shared<TestWindow>()};
        FakeSystemWindow            sys{window, 800, 600};

        Div &viewport() {
            return window->appContainer().add<Div>().position(Position::Absolute).left(0).top(0).width(200).height(200).overflow(Overflow::Scroll);
        }

        void wheel(double scrollY) {
            window->handleMouseScroll({.x = 50, .y = 50, .scrollY = scrollY});
        }
    };
}

// C5: Div.cpp doesn't correct the scroll offset when the content shrinks
TEST_CASE("C5: the scroll offset follows content that shrinks", "[div][scroll][!shouldfail]") {
    Scrolling f;
    auto     &viewport = f.viewport();
    auto     &content  = viewport.add<Div>().width(200).height(1000).shrink(0);
    f.sys.layout();
    viewport.scrollBy(0, -800);
    REQUIRE(viewport.scrollY() == -800);
    content.height(100);
    f.sys.layout();
    CHECK(viewport.scrollY() == 0);
    f.wheel(3);
    CHECK(viewport.scrollY() == 0);
}

// C6: a wheel delta becomes ceil(delta) * 2 px
TEST_CASE("C6: fractional wheel deltas scroll both ways alike", "[div][scroll][!shouldfail]") {
    Scrolling f;
    auto     &viewport = f.viewport();
    viewport.add<Div>().width(200).height(1000).shrink(0);
    f.sys.layout();
    viewport.scrollBy(0, -400);
    f.wheel(-0.5);
    const int down = -400 - viewport.scrollY();
    CHECK(down > 0);
    f.wheel(0.5);
    CHECK(viewport.scrollY() == -400);
}

TEST_CASE("C6: a wheel notch scrolls more than 2 px", "[div][scroll][!shouldfail]") {
    // How far is to be decided (browsers move about 40 to 100 px); 2 px is a tenth of a line
    Scrolling f;
    auto     &viewport = f.viewport();
    viewport.add<Div>().width(200).height(1000).shrink(0);
    f.sys.layout();
    f.wheel(-1);
    CHECK(viewport.scrollY() < -2);
}

// C7: every scroller on the path takes the wheel event
TEST_CASE("C7: a wheel event scrolls the innermost scroller only", "[div][scroll][!shouldfail]") {
    Scrolling f;
    auto     &outer = f.viewport();
    auto     &inner = outer.add<Div>().width(200).height(100).shrink(0).overflow(Overflow::Scroll);
    inner.add<Div>().width(200).height(1000).shrink(0);
    outer.add<Div>().width(200).height(1000).shrink(0);
    f.sys.layout();
    f.wheel(-3);
    CHECK(inner.scrollY() < 0);
    CHECK(outer.scrollY() == 0);
}
