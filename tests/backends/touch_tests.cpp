#include <catch2/catch_test_macros.hpp>
#include "fake_system_window.hpp"

using namespace psychic_ui;
using namespace psychic_ui_tests;

TEST_CASE("The window starts in pointer mode and reports changes once", "[backend][touch]") {
    auto window = std::make_shared<TestWindow>();
    FakeSystemWindow systemWindow(window);
    CHECK(window->inputMode() == InputMode::Pointer);

    std::vector<InputMode> modes{};
    window->onInputModeChange([&](InputMode mode) { modes.push_back(mode); });

    window->handleInputModeChange(InputMode::Touch);
    window->handleInputModeChange(InputMode::Touch);
    window->inputMode(InputMode::Pen).inputMode(InputMode::Pointer);
    CHECK(modes == std::vector{InputMode::Touch, InputMode::Pen, InputMode::Pointer});
    CHECK(window->inputMode() == InputMode::Pointer);
}

TEST_CASE("The :touch pseudo-class follows the input mode", "[backend][touch][style]") {
    auto window = std::make_shared<TestWindow>();
    FakeSystemWindow systemWindow(window);
    auto &div = window->appContainer().add<Div>().addClassName("probe");
    window->styleManager()->style(".probe").padding(1);
    window->styleManager()->style("Window:touch .probe").padding(4);
    window->styleManager()->style(".probe:touch").paddingLeft(8);
    systemWindow.layout();
    CHECK(div.computedStyle().padding() == Length{1});
    CHECK(div.computedStyle().paddingLeft() == Length{});

    window->handleInputModeChange(InputMode::Touch);
    systemWindow.layout();
    CHECK(div.computedStyle().padding() == Length{4});
    CHECK(div.computedStyle().paddingLeft() == Length{8});

    // A pen isn't touch
    window->handleInputModeChange(InputMode::Pen);
    systemWindow.layout();
    CHECK(div.computedStyle().padding() == Length{1});
}

namespace {
    using namespace std::chrono_literals;

    /**
     * A window on a fake system window, with a 100 x 100 div at (0, 0) of the app container
     */
    struct TouchFixture {
        std::shared_ptr<TestWindow> window{std::make_shared<TestWindow>()};
        FakeSystemWindow            systemWindow{window};
        Div                         &target{window->appContainer().add<Div>()};

        int                      presses{0};
        int                      releases{0};
        int                      clicks{0};
        std::vector<MouseButton> buttons{};
        int                      pressX{-1};
        int                      pressY{-1};
        int                      moves{0};

        TouchFixture() {
            target.width(100.0f).height(100.0f);
            systemWindow.layout();
            target.onMousePress([this](int x, int y, MouseButton button, Mod) {
                ++presses;
                buttons.push_back(button);
                pressX = x;
                pressY = y;
            });
            target.onMouseRelease([this](int, int, MouseButton, Mod) { ++releases; });
            target.onClick([this]() { ++clicks; });
            window->onMouseMove([this](int, int, int, Mod) { ++moves; });
        }
    };

    /**
     * A 100 x 100 scrollable div at (0, 0), over content of the given height
     */
    struct ScrollFixture {
        std::shared_ptr<TestWindow> window{std::make_shared<TestWindow>()};
        FakeSystemWindow            systemWindow{window};
        Div                         &viewport{window->appContainer().add<Div>()};
        Div                         &content{viewport.add<Div>()};
        int                         contentPresses{0};
        std::chrono::nanoseconds    time{1s};

        explicit ScrollFixture(float height = 1000.0f) {
            viewport.width(100.0f).height(100.0f).overflow(Overflow::Scroll);
            content.width(100.0f).height(height).shrink(0);
            content.onMousePress([this](int, int, MouseButton, Mod) { ++contentPresses; });
            systemWindow.layout();
        }

        /**
         * Up by 10 px every 10 ms (1000 px/s), from (50, 90) to (50, 30), and lift
         */
        void swipeUp() {
            window->handleTouchPress(50.0f, 90.0f, time);
            for (int i = 1; i <= 6; ++i) {
                time += 10ms;
                window->handleTouchMove(50.0f, 90.0f - 10.0f * static_cast<float>(i), time);
            }
            window->handleTouchRelease(50.0f, 30.0f, time);
        }
    };
}

TEST_CASE("Touch: a tap clicks once where the finger landed", "[backend][touch]") {
    TouchFixture f;
    f.window->handleTouchPress(20.5f, 30.5f, 1s);
    CHECK(f.window->inputMode() == InputMode::Touch);
    CHECK(f.presses == 0); // Could still become a drag, a pan or a long press
    f.window->handleTouchMove(23.0f, 32.0f, 1s + 50ms); // Within the slop
    f.window->handleTouchRelease(23.0f, 32.0f, 1s + 100ms);
    CHECK(f.presses == 1);
    int originX = 0;
    int originY = 0;
    f.target.localToGlobal(originX, originY);
    CHECK(f.pressX == 20 - originX);
    CHECK(f.pressY == 30 - originY);
    CHECK(f.releases == 1);
    CHECK(f.clicks == 1);
    CHECK(f.buttons == std::vector{MouseButton::LEFT});
    CHECK_FALSE(f.target.mouseOver()); // No hover left behind
}

TEST_CASE("Touch: a drag starts past the slop, from where the finger landed", "[backend][touch]") {
    TouchFixture f;
    f.window->handleTouchPress(20.0f, 20.0f, 1s);
    f.window->handleTouchMove(25.0f, 20.0f, 1s + 10ms);
    CHECK(f.presses == 0);
    const int movesBefore = f.moves;
    f.window->handleTouchMove(40.0f, 20.0f, 1s + 20ms);
    CHECK(f.presses == 1);
    CHECK(f.pressX < 20); // Where it landed, in the div
    CHECK(f.moves > movesBefore);
    f.window->handleTouchMove(150.0f, 20.0f, 1s + 30ms); // Outside: the drag goes on
    f.window->handleTouchRelease(150.0f, 20.0f, 1s + 40ms);
    CHECK(f.releases == 1);
    CHECK(f.clicks == 0); // Released outside
}

TEST_CASE("Touch: a long press is a right button press", "[backend][touch]") {
    TouchFixture f;
    f.window->handleTouchPress(20.0f, 20.0f, 1s);
    f.window->updateGestures(1s + 499ms);
    CHECK(f.presses == 0);
    f.window->updateGestures(1s + 500ms);
    CHECK(f.presses == 1);
    CHECK(f.buttons == std::vector{MouseButton::RIGHT});
    CHECK(f.releases == 0); // Until the finger lifts
    f.window->handleTouchRelease(20.0f, 20.0f, 2s);
    CHECK(f.releases == 1);
    CHECK(f.buttons == std::vector{MouseButton::RIGHT}); // No tap after it
}

TEST_CASE("Touch: a long press is also noticed when the finger lifts", "[backend][touch]") {
    TouchFixture f;
    f.window->handleTouchPress(20.0f, 20.0f, 1s);
    f.window->handleTouchRelease(20.0f, 20.0f, 2s); // No frame in between
    CHECK(f.buttons == std::vector{MouseButton::RIGHT});
}

TEST_CASE("Touch: a cancelled drag releases without clicking", "[backend][touch]") {
    TouchFixture f;
    f.window->handleTouchPress(20.0f, 20.0f, 1s);
    f.window->handleTouchMove(40.0f, 20.0f, 1s + 10ms);
    f.window->handleTouchMove(30.0f, 20.0f, 1s + 20ms);
    f.window->handleTouchCancel(1s + 30ms);
    CHECK(f.presses == 1);
    CHECK(f.releases == 1);
    CHECK(f.clicks == 0);
}

TEST_CASE("Touch: a finger drag pans, kinetic scrolling decelerates and a touch stops it", "[backend][touch][scroll]") {
    ScrollFixture f;
    REQUIRE(f.viewport.contentHeight() == 1000);

    f.swipeUp();
    CHECK(f.viewport.scrollY() == -60); // The content followed the finger
    CHECK(f.contentPresses == 0);       // A pan presses nothing
    REQUIRE(f.window->flinging());

    // Every frame moves less than the one before
    std::vector<int> steps{};
    int              last = f.viewport.scrollY();
    for (int frame = 0; frame < 5; ++frame) {
        f.time += 16ms;
        f.window->updateGestures(f.time);
        steps.push_back(last - f.viewport.scrollY());
        last = f.viewport.scrollY();
    }
    CHECK(steps.front() > 0);
    CHECK(steps.front() <= 16); // About 1000 px/s
    CHECK(steps.back() < steps.front());
    for (std::size_t i = 1; i < steps.size(); ++i) {
        CHECK(steps[i] <= steps[i - 1]);
    }

    // A finger stops it, and isn't a tap
    f.window->handleTouchPress(50.0f, 50.0f, f.time);
    CHECK_FALSE(f.window->flinging());
    const int stopped = f.viewport.scrollY();
    f.time += 16ms;
    f.window->updateGestures(f.time);
    f.window->handleTouchRelease(50.0f, 50.0f, f.time);
    CHECK(f.viewport.scrollY() == stopped);
    CHECK(f.contentPresses == 0);
}

TEST_CASE("Touch: kinetic scrolling stops by itself", "[backend][touch][scroll]") {
    ScrollFixture f(100000.0f);
    f.swipeUp();
    REQUIRE(f.window->flinging());
    int frames = 0;
    while (f.window->flinging() && frames < 1000) {
        f.time += 16ms;
        f.window->updateGestures(f.time);
        ++frames;
    }
    CHECK_FALSE(f.window->flinging());
    CHECK(frames < 300); // A few seconds
    // About v0 / rate (1000 px/s over -ln(0.998) * 1000 per second: 500 px), after the 60 px panned
    CHECK(f.viewport.scrollY() < -400);
    CHECK(f.viewport.scrollY() > -600);
}

TEST_CASE("Touch: kinetic scrolling stops at the end of the content", "[backend][touch][scroll]") {
    ScrollFixture f(200.0f);
    f.swipeUp();
    REQUIRE(f.window->flinging());
    for (int frame = 0; frame < 20 && f.window->flinging(); ++frame) {
        f.time += 16ms;
        f.window->updateGestures(f.time);
    }
    CHECK_FALSE(f.window->flinging());
    CHECK(f.viewport.scrollY() == -100);
}

TEST_CASE("Touch: a pan that rests before lifting doesn't fling", "[backend][touch][scroll]") {
    ScrollFixture f;
    f.window->handleTouchPress(50.0f, 90.0f, 1s);
    f.window->handleTouchMove(50.0f, 60.0f, 1s + 30ms);
    f.window->handleTouchRelease(50.0f, 60.0f, 1s + 300ms);
    CHECK(f.viewport.scrollY() == -30);
    CHECK_FALSE(f.window->flinging());
}

TEST_CASE("Touch: a drag across a vertical scroller drags what is under the finger", "[backend][touch][scroll]") {
    ScrollFixture f;
    f.window->handleTouchPress(50.0f, 50.0f, 1s);
    f.window->handleTouchMove(80.0f, 52.0f, 1s + 20ms); // Mostly horizontal: nothing scrolls that way
    CHECK(f.contentPresses == 1);
    CHECK(f.viewport.scrollY() == 0);
    f.window->handleTouchRelease(80.0f, 52.0f, 1s + 40ms);
}
