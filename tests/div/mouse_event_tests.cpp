#include <chrono> // IWYU pragma: keep: the chrono literals (500ms), which clang-tidy maps to <bits/chrono.h> only
#include <memory>
#include <vector>
#include <catch2/catch_test_macros.hpp>
#include <psychic-ui/Div.hpp>
#include <psychic-ui/components/Button.hpp>
#include <psychic-ui/psychic-ui.hpp>
#include <psychic-ui/style/Values.hpp>
#include "../backends/fake_system_window.hpp"

using namespace psychic_ui;
using namespace psychic_ui_tests;
using namespace std::chrono_literals;

namespace {
    constexpr unsigned left  = static_cast<unsigned>(MouseButton::LEFT);
    constexpr unsigned right = static_cast<unsigned>(MouseButton::RIGHT);

    /**
     * A 100 x 100 box at (20, 30) of the window, with a 20 x 20 child at (10, 10) of the box
     */
    struct MouseFixture {
        std::shared_ptr<TestWindow> window{std::make_shared<TestWindow>()};
        FakeSystemWindow            systemWindow{window};
        Div                         &box{window->appContainer().add<Div>()};
        Div                         &inner{box.add<Div>()};

        MouseFixture() {
            box.position(Position::Absolute).width(100).height(100);
            inner.position(Position::Absolute).left(10).top(10).width(20).height(20);
            systemWindow.layout();
            // Wherever the app container is in the window
            int appX = 0;
            int appY = 0;
            window->appContainer().localToGlobal(appX, appY);
            box.left(static_cast<float>(20 - appX)).top(static_cast<float>(30 - appY));
            systemWindow.layout();
            int boxX = 0;
            int boxY = 0;
            box.localToGlobal(boxX, boxY);
            REQUIRE(boxX == 20);
            REQUIRE(boxY == 30);
        }

        void press(float x, float y, MouseButton button = MouseButton::LEFT, Mod modifiers = {}) const {
            window->handleMouseButtonChange({.x = x, .y = y, .button = button, .down = true, .modifiers = modifiers});
        }

        void release(float x, float y, MouseButton button = MouseButton::LEFT, Mod modifiers = {}) const {
            window->handleMouseButtonChange({.x = x, .y = y, .button = button, .down = false, .modifiers = modifiers});
        }

        void key(Key key, Mod modifiers = {}) const {
            window->handleKeyPress(key, modifiers);
        }
    };
}

TEST_CASE("Mouse events: every signal gets the event in the div's own coordinates", "[div][events][mouse]") {
    MouseFixture fixture;
    std::vector<MouseEvent> entered{};
    std::vector<MouseEvent> moved{};
    std::vector<MouseEvent> pressed{};
    std::vector<MouseEvent> released{};
    std::vector<MouseEvent> changed{};
    std::vector<MouseEvent> clicked{};
    fixture.box.onMouseEnter([&](const MouseEvent &event) { entered.push_back(event); })
         .onMouseMove([&](const MouseEvent &event) { moved.push_back(event); })
         .onMousePress([&](const MouseEvent &event) { pressed.push_back(event); })
         .onMouseRelease([&](const MouseEvent &event) { released.push_back(event); })
         .onMouseButtonChange([&](const MouseEvent &event) { changed.push_back(event); })
         .onClick([&](const MouseEvent &event) { clicked.push_back(event); });

    fixture.window->handleMouseMove({.x = 70.5f, .y = 80.0f, .modifiers = {.shift = true}}, false);
    REQUIRE(entered.size() == 1);
    REQUIRE(moved.size() == 1);
    CHECK(moved[0].x == 50.5f);
    CHECK(moved[0].y == 50.0f);
    CHECK(moved[0].windowX == 70.5f);
    CHECK(moved[0].windowY == 80.0f);
    CHECK(moved[0].button == MouseButton::NONE);
    CHECK(moved[0].buttons == 0);
    CHECK(moved[0].modifiers.shift);
    CHECK(moved[0].clickCount == 0);
    CHECK(moved[0].source == InputMode::Pointer);
    CHECK(entered[0].x == 50.5f);

    fixture.press(70.5f, 80.0f, MouseButton::LEFT, {.ctrl = true});
    REQUIRE(pressed.size() == 1);
    CHECK(pressed[0].x == 50.5f);
    CHECK(pressed[0].y == 50.0f);
    CHECK(pressed[0].windowX == 70.5f);
    CHECK(pressed[0].button == MouseButton::LEFT);
    CHECK(pressed[0].down);
    CHECK(pressed[0].buttons == left);
    CHECK(pressed[0].modifiers.ctrl);
    CHECK(pressed[0].clickCount == 1);

    // A second button while the first is held
    fixture.press(70.5f, 80.0f, MouseButton::RIGHT);
    REQUIRE(pressed.size() == 2);
    CHECK(pressed[1].button == MouseButton::RIGHT);
    CHECK(pressed[1].buttons == (left | right));
    fixture.window->handleMouseMove({.x = 72.0f, .y = 80.0f}, false);
    CHECK(moved.back().buttons == (left | right));
    // Left first: a div's pressed state doesn't tell the buttons apart, any release ends it
    fixture.release(72.0f, 80.0f, MouseButton::LEFT);
    fixture.release(72.0f, 80.0f, MouseButton::RIGHT);

    REQUIRE(released.size() == 2);
    CHECK(released[0].button == MouseButton::LEFT);
    CHECK_FALSE(released[0].down);
    CHECK(released[0].buttons == right);
    CHECK(released[0].x == 52.0f);
    CHECK(released[1].buttons == 0);

    REQUIRE(clicked.size() == 1); // Left button only
    CHECK(clicked[0].button == MouseButton::LEFT);
    CHECK(clicked[0].x == 52.0f);
    CHECK(clicked[0].windowY == 80.0f);
    CHECK(clicked[0].clickCount == 1);

    REQUIRE(changed.size() == 4);
    CHECK(changed[0].down);
    CHECK(changed[1].down);
    CHECK_FALSE(changed[2].down);
    CHECK_FALSE(changed[3].down);

    // The child gets its own coordinates, the window ones stay
    std::vector<MouseEvent> innerPressed{};
    fixture.inner.onMousePress([&](const MouseEvent &event) { innerPressed.push_back(event); });
    fixture.press(35.0f, 45.0f);
    REQUIRE(innerPressed.size() == 1);
    CHECK(innerPressed[0].x == 5.0f);
    CHECK(innerPressed[0].y == 5.0f);
    CHECK(innerPressed[0].windowX == 35.0f);
    CHECK(innerPressed[0].windowY == 45.0f);
    fixture.release(35.0f, 45.0f);

    // Leaving
    std::vector<MouseEvent> exited{};
    fixture.box.onMouseExit([&](const MouseEvent &event) { exited.push_back(event); });
    fixture.window->handleMouseMove({.x = 300.0f, .y = 300.0f}, false);
    REQUIRE(exited.size() == 1);
    CHECK(exited[0].windowX == 300.0f);
}

TEST_CASE("Mouse events: presses in a row count clicks", "[div][events][mouse]") {
    MouseFixture fixture;
    std::vector<unsigned> presses{};
    std::vector<unsigned> clicks{};
    std::vector<unsigned> doubleClicks{};
    fixture.box.onMousePress([&](const MouseEvent &event) { presses.push_back(event.clickCount); })
         .onClick([&](const MouseEvent &event) { clicks.push_back(event.clickCount); })
         .onDoubleClick([&](const MouseEvent &event) { doubleClicks.push_back(event.clickCount); });

    for (int i = 0; i < 3; ++i) {
        fixture.press(70.0f, 80.0f);
        fixture.release(70.0f, 80.0f);
    }
    CHECK(presses == std::vector<unsigned>{1, 2, 3});
    CHECK(clicks == std::vector<unsigned>{1, 2, 3});
    CHECK(doubleClicks == std::vector<unsigned>{2, 3});

    // Another button starts over, and so does a press elsewhere
    fixture.press(70.0f, 80.0f, MouseButton::RIGHT);
    fixture.release(70.0f, 80.0f, MouseButton::RIGHT);
    fixture.press(70.0f, 80.0f);
    fixture.release(70.0f, 80.0f);
    fixture.press(90.0f, 80.0f);
    fixture.release(90.0f, 80.0f);
    CHECK(presses == std::vector<unsigned>{1, 2, 3, 1, 1, 1});
    CHECK(doubleClicks == std::vector<unsigned>{2, 3});

    // A backend's own count is kept
    fixture.window->handleMouseButtonChange({.x = 90.0f, .y = 80.0f, .button = MouseButton::LEFT, .down = true, .clickCount = 2});
    fixture.release(90.0f, 80.0f);
    CHECK(presses.back() == 2);
    CHECK(clicks.back() == 2);
    CHECK(doubleClicks == std::vector<unsigned>{2, 3, 2});
}

TEST_CASE("Mouse events: scrolls carry their modifiers", "[div][events][mouse]") {
    MouseFixture fixture;
    std::vector<MouseEvent> scrolls{};
    fixture.box.onMouseScroll([&](const MouseEvent &event) { scrolls.push_back(event); });
    fixture.window->handleMouseScroll({.x = 70.0f, .y = 80.0f, .modifiers = {.shift = true}, .scrollX = 0.5, .scrollY = -2.0});
    REQUIRE(scrolls.size() == 1);
    CHECK(scrolls[0].x == 50.0f);
    CHECK(scrolls[0].windowX == 70.0f);
    CHECK(scrolls[0].modifiers.shift);
    CHECK_FALSE(scrolls[0].modifiers.ctrl);
    CHECK(scrolls[0].scrollX == 0.5);
    CHECK(scrolls[0].scrollY == -2.0);
    CHECK(scrolls[0].button == MouseButton::NONE);
}

TEST_CASE("Mouse events: touches report their source and count taps on the touch clock", "[div][events][mouse][touch]") {
    MouseFixture fixture;
    std::vector<MouseEvent> entered{};
    std::vector<MouseEvent> pressed{};
    std::vector<MouseEvent> clicked{};
    unsigned doubleTaps = 0;
    fixture.box.onMouseEnter([&](const MouseEvent &event) { entered.push_back(event); })
         .onMousePress([&](const MouseEvent &event) { pressed.push_back(event); })
         .onClick([&](const MouseEvent &event) { clicked.push_back(event); })
         .onDoubleClick([&] { ++doubleTaps; });

    fixture.window->handleTouchPress(70.0f, 80.0f, 1s);
    fixture.window->handleTouchRelease(70.0f, 80.0f, 1s + 50ms);
    REQUIRE(pressed.size() == 1);
    CHECK(pressed[0].source == InputMode::Touch);
    CHECK(pressed[0].x == 50.0f);
    CHECK(pressed[0].windowX == 70.0f);
    CHECK(pressed[0].clickCount == 1);
    REQUIRE(entered.size() == 1);
    CHECK(entered[0].source == InputMode::Touch);
    REQUIRE(clicked.size() == 1);
    CHECK(clicked[0].source == InputMode::Touch);

    // A second tap soon after, a finger's width away, is a double tap
    fixture.window->handleTouchPress(76.0f, 84.0f, 1s + 250ms);
    fixture.window->handleTouchRelease(76.0f, 84.0f, 1s + 300ms);
    REQUIRE(pressed.size() == 2);
    CHECK(pressed[1].clickCount == 2);
    CHECK(doubleTaps == 1);

    // Much later, by the touch clock, it starts over
    fixture.window->handleTouchPress(76.0f, 84.0f, 5s);
    fixture.window->handleTouchRelease(76.0f, 84.0f, 5s + 50ms);
    REQUIRE(pressed.size() == 3);
    CHECK(pressed[2].clickCount == 1);
    CHECK(doubleTaps == 1);
}

TEST_CASE("Context menus: a right press asks once, the deepest div that listens gets it", "[div][events][mouse][context-menu]") {
    MouseFixture fixture;
    std::vector<MouseEvent> innerRequests{};
    std::vector<MouseEvent> boxRequests{};
    fixture.inner.onContextMenuRequest([&](const MouseEvent &event) { innerRequests.push_back(event); });
    fixture.box.onContextMenuRequest([&](const MouseEvent &event) { boxRequests.push_back(event); });

    fixture.press(35.0f, 45.0f, MouseButton::RIGHT, {.shift = true});
    REQUIRE(innerRequests.size() == 1);
    CHECK(innerRequests[0].x == 5.0f);
    CHECK(innerRequests[0].windowX == 35.0f);
    CHECK(innerRequests[0].button == MouseButton::RIGHT);
    CHECK(innerRequests[0].modifiers.shift);
    CHECK(innerRequests[0].clickCount == 1);
    CHECK(innerRequests[0].source == InputMode::Pointer);
    fixture.release(35.0f, 45.0f, MouseButton::RIGHT);
    CHECK(innerRequests.size() == 1); // Not again on the release
    CHECK(boxRequests.empty());

    // Outside the child, the box
    fixture.press(100.0f, 100.0f, MouseButton::RIGHT);
    fixture.release(100.0f, 100.0f, MouseButton::RIGHT);
    REQUIRE(boxRequests.size() == 1);
    CHECK(boxRequests[0].x == 80.0f);

    // Other buttons don't ask
    fixture.press(35.0f, 45.0f);
    fixture.release(35.0f, 45.0f);
    fixture.press(35.0f, 45.0f, MouseButton::MIDDLE);
    fixture.release(35.0f, 45.0f, MouseButton::MIDDLE);
    CHECK(innerRequests.size() == 1);
    CHECK(boxRequests.size() == 1);
}

TEST_CASE("Context menus: a touch long press asks once", "[div][events][mouse][context-menu][touch]") {
    MouseFixture fixture;
    std::vector<MouseEvent> requests{};
    fixture.box.onContextMenuRequest([&](const MouseEvent &event) { requests.push_back(event); });

    fixture.window->handleTouchPress(70.0f, 80.0f, 1s);
    fixture.window->updateGestures(1s + 600ms);
    fixture.window->updateGestures(1s + 700ms);
    REQUIRE(requests.size() == 1);
    CHECK(requests[0].source == InputMode::Touch);
    CHECK(requests[0].button == MouseButton::RIGHT);
    CHECK(requests[0].x == 50.0f);
    fixture.window->handleTouchRelease(70.0f, 80.0f, 2s);
    CHECK(requests.size() == 1);

    // Lifting before the delay is a tap, which doesn't ask
    fixture.window->handleTouchPress(70.0f, 80.0f, 3s);
    fixture.window->handleTouchRelease(70.0f, 80.0f, 3s + 100ms);
    CHECK(requests.size() == 1);
}

TEST_CASE("Context menus: the Menu key and Shift+F10 ask at the focused div's center", "[div][events][context-menu][keyboard]") {
    MouseFixture fixture;
    std::vector<MouseEvent> requests{};
    int                     keys = 0;
    fixture.inner.onContextMenuRequest([&](const MouseEvent &event) { requests.push_back(event); })
           .onKeyPress([&] { ++keys; });
    fixture.window->requestFocus(&fixture.inner);

    fixture.key(Key::MENU);
    REQUIRE(requests.size() == 1);
    CHECK(requests[0].x == 10.0f);
    CHECK(requests[0].y == 10.0f);
    CHECK(requests[0].windowX == 40.0f);
    CHECK(requests[0].windowY == 50.0f);
    CHECK(requests[0].button == MouseButton::NONE);
    CHECK(requests[0].clickCount == 0); // Not made by a press
    CHECK(keys == 0); // Taken by the request

    fixture.key(Key::F10, {.shift = true});
    CHECK(requests.size() == 2);

    // Not with other modifiers, nor F10 alone: those are keys like the others
    fixture.key(Key::F10);
    fixture.key(Key::MENU, {.ctrl = true});
    fixture.key(Key::F10, {.shift = true, .alt = true});
    CHECK(requests.size() == 2);
    CHECK(keys == 3);

    // An ancestor that listens gets it at the focused div's center, in its own coordinates
    std::vector<MouseEvent> boxRequests{};
    fixture.box.onContextMenuRequest([&](const MouseEvent &event) { boxRequests.push_back(event); });
    auto &plain = fixture.box.add<Div>();
    plain.position(Position::Absolute).left(50).top(50).width(40).height(20);
    fixture.systemWindow.layout();
    fixture.window->requestFocus(&plain);
    fixture.key(Key::MENU);
    REQUIRE(boxRequests.size() == 1);
    CHECK(boxRequests[0].x == 70.0f);
    CHECK(boxRequests[0].y == 60.0f);
    CHECK(boxRequests[0].windowX == 90.0f);
    CHECK(requests.size() == 2);

    // Nothing listening: the key goes on as a key
    fixture.window->requestFocus(nullptr);
    CHECK_FALSE(fixture.window->handleKeyPress(Key::MENU, {}));
}

TEST_CASE("Mouse events: a button clicked from the keyboard clicks at its center", "[div][events][keyboard]") {
    MouseFixture fixture;
    std::vector<MouseEvent> clicks{};
    auto &button = fixture.box.add<Button>("Go");
    button.position(Position::Absolute).left(50).top(40).width(40).height(20);
    button.onClick([&](const MouseEvent &event) { clicks.push_back(event); });
    fixture.systemWindow.layout();
    fixture.window->requestFocus(&button);
    fixture.key(Key::ENTER);
    REQUIRE(clicks.size() == 1);
    CHECK(clicks[0].x == 20.0f);
    CHECK(clicks[0].windowX == 90.0f);
    CHECK(clicks[0].windowY == 80.0f);
    CHECK(clicks[0].button == MouseButton::LEFT);
    CHECK(clicks[0].clickCount == 0);
}
