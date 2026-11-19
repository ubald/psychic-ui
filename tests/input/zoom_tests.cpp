#include <memory>
#include <vector>
#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include "../backends/fake_system_window.hpp"
#include <psychic-ui/Div.hpp>
#include <psychic-ui/Window.hpp>
#include <psychic-ui/psychic-ui.hpp>
#include <psychic-ui/style/Values.hpp>

using namespace psychic_ui;
using namespace psychic_ui_tests;

namespace {
    struct Zooming {
        std::shared_ptr<TestWindow> window{std::make_shared<TestWindow>()};
        FakeSystemWindow            sys{window, 800, 600};

        static Div &box(Div &parent, float left, float top, float width, float height) {
            return parent.add<Div>().position(Position::Absolute).left(left).top(top).width(width).height(height);
        }

        Div &box(float left, float top, float width, float height) {
            return box(window->appContainer(), left, top, width, height);
        }
    };
}

TEST_CASE("ctrl + wheel zooms the div under the pointer and doesn't scroll", "[zoom]") {
    Zooming fixture;
    auto   &scroller = fixture.box(0, 0, 200, 200).overflow(Overflow::Scroll);
    auto   &content  = scroller.add<Div>().width(200).height(1000).shrink(0);
    fixture.sys.layout();

    std::vector<ZoomEvent> zooms{};
    content.onZoom([&](const ZoomEvent &event) { zooms.push_back(event); });

    fixture.window->handleMouseScroll({.x = 50, .y = 60, .modifiers = {.ctrl = true}, .scrollY = 1});
    fixture.window->handleMouseScroll({.x = 50, .y = 60, .modifiers = {.ctrl = true}, .scrollY = -1});
    REQUIRE(zooms.size() == 2);
    CHECK(zooms[0].scale == Catch::Approx(Window::wheelZoomStep));
    CHECK(zooms[1].scale == Catch::Approx(1.0f / Window::wheelZoomStep));
    CHECK(zooms[0].windowX == 50);
    CHECK(zooms[0].windowY == 60);
    CHECK(zooms[0].modifiers.ctrl);
    CHECK(zooms[0].source == InputMode::Pointer);
    CHECK(scroller.scrollY() == 0);

    // Two notches at once zoom twice as far
    fixture.window->handleMouseScroll({.x = 50, .y = 60, .modifiers = {.ctrl = true}, .scrollY = 2});
    REQUIRE(zooms.size() == 3);
    CHECK(zooms[2].scale == Catch::Approx(Window::wheelZoomStep * Window::wheelZoomStep));

    // Without Ctrl, the wheel still scrolls
    fixture.window->handleMouseScroll({.x = 50, .y = 60, .scrollY = -3});
    CHECK(zooms.size() == 3);
    CHECK(scroller.scrollY() < 0);
}

TEST_CASE("a zoom no div takes does nothing", "[zoom]") {
    Zooming fixture;
    auto   &scroller = fixture.box(0, 0, 200, 200).overflow(Overflow::Scroll);
    scroller.add<Div>().width(200).height(1000).shrink(0);
    fixture.sys.layout();

    std::vector<ZoomEvent> zooms{};
    auto &elsewhere = fixture.box(300, 300, 100, 100);
    elsewhere.onZoom([&](const ZoomEvent &event) { zooms.push_back(event); });
    fixture.sys.layout();

    fixture.window->handleZoom({.x = 50, .y = 50, .scale = 1.5f});
    fixture.window->handleMouseScroll({.x = 50, .y = 50, .modifiers = {.ctrl = true}, .scrollY = -1});
    CHECK(zooms.empty());
    CHECK(scroller.scrollY() == 0);
}

TEST_CASE("zoom coordinates are the receiving div's", "[zoom]") {
    Zooming fixture;
    auto   &outer = fixture.box(100, 50, 300, 300);
    auto   &inner = fixture.box(outer, 20, 30, 100, 100);
    fixture.sys.layout();

    std::vector<ZoomEvent> zooms{};
    inner.onZoom([&](const ZoomEvent &event) { zooms.push_back(event); });

    int originX = 0;
    int originY = 0;
    inner.localToGlobal(originX, originY);
    fixture.window->handleZoom({.x = 150, .y = 110, .scale = 1.25f, .panX = 3, .panY = -2});
    REQUIRE(zooms.size() == 1);
    CHECK(zooms[0].x == static_cast<float>(150 - originX));
    CHECK(zooms[0].y == static_cast<float>(110 - originY));
    CHECK(zooms[0].windowX == 150);
    CHECK(zooms[0].windowY == 110);
    CHECK(zooms[0].scale == Catch::Approx(1.25f));
    CHECK(zooms[0].panX == 3);
    CHECK(zooms[0].panY == -2);
}

TEST_CASE("a zoom goes to the deepest subscriber first", "[zoom]") {
    Zooming fixture;
    auto   &outer = fixture.box(0, 0, 300, 300);
    auto   &inner = fixture.box(outer, 0, 0, 100, 100);
    fixture.sys.layout();

    int  outerZooms = 0;
    int  innerZooms = 0;
    bool innerUses  = true;
    outer.onZoom([&] { ++outerZooms; });
    inner.onZoom([&](const ZoomEvent &) {
        ++innerZooms;
        return innerUses;
    });

    fixture.window->handleZoom({.x = 50, .y = 50, .scale = 1.1f});
    CHECK(innerZooms == 1);
    CHECK(outerZooms == 0);

    // A callback returning false passes it to the next div up
    innerUses = false;
    fixture.window->handleZoom({.x = 50, .y = 50, .scale = 1.1f});
    CHECK(innerZooms == 2);
    CHECK(outerZooms == 1);

    // Outside the inner div, the outer one gets it
    fixture.window->handleZoom({.x = 200, .y = 200, .scale = 1.1f});
    CHECK(innerZooms == 2);
    CHECK(outerZooms == 2);
}
