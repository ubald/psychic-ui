#include <chrono>
#include <cstddef>
#include <memory>
#include <vector>
#include <catch2/catch_test_macros.hpp>
#include <psychic-ui/components/Button.hpp>
#include <psychic-ui/components/CheckBox.hpp>
#include <psychic-ui/components/Range.hpp>
#include <psychic-ui/psychic-ui.hpp>
#include <psychic-ui/style/StyleManager.hpp>
#include <psychic-ui/style/StyleSheet.hpp>
#include <psychic-ui/style/Values.hpp>
#include <psychic-ui/themes/Theme.hpp>
#include <psychic-ui/themes/one-dark.hpp>
#include "fake_system_window.hpp"

using namespace psychic_ui;
using namespace psychic_ui_tests;

TEST_CASE("The window starts in pointer mode and reports changes once", "[backend][touch]") {
    auto window = std::make_shared<TestWindow>();
    const FakeSystemWindow systemWindow(window);
    CHECK(window->inputMode() == InputMode::Pointer);

    std::vector<InputMode> modes{};
    window->onInputModeChange([&](InputMode mode) { modes.push_back(mode); });

    window->handleInputModeChange(InputMode::Touch);
    window->handleInputModeChange(InputMode::Touch);
    window->inputMode(InputMode::Pen).inputMode(InputMode::Pointer);
    CHECK(modes == std::vector{InputMode::Touch, InputMode::Pen, InputMode::Pointer});
    CHECK(window->inputMode() == InputMode::Pointer);
}

namespace {
    /**
     * In a stylesheet, which a density switch loads again
     */
    struct ProbeSheet : StyleSheet {
        void load(StyleManager *manager) override {
            manager->style(".probe").padding(1);
            manager->style("Window:touch .probe").padding(4);
            manager->style(".probe:touch").paddingLeft(8);
        }
    };
}

TEST_CASE("The :touch pseudo-class follows the input mode", "[backend][touch][style]") {
    auto window = std::make_shared<TestWindow>();
    FakeSystemWindow systemWindow(window);
    auto &div = window->appContainer().add<Div>().addClassName("probe");
    window->loadStyleSheet<ProbeSheet>();
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
            target.onMousePress([this](const MouseEvent &event) {
                ++presses;
                buttons.push_back(event.button);
                pressX = static_cast<int>(event.x);
                pressY = static_cast<int>(event.y);
            });
            target.onMouseRelease([this]() { ++releases; });
            target.onClick([this]() { ++clicks; });
            window->onMouseMove([this]() { ++moves; });
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
            content.onMousePress([this]() { ++contentPresses; });
            systemWindow.layout();
        }

        /**
         * Up by 10 px every 10 ms (1000 px/s), from (50, 90) to (50, 30), and lift
         */
        void swipeUp() {
            window->handleTouchPress(50.0f, 90.0f, time);
            for (int i = 1; i <= 6; ++i) {
                time += 10ms;
                window->handleTouchMove(50.0f, 90.0f - (10.0f * static_cast<float>(i)), time);
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
    const TouchFixture f;
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
    CHECK(f.clicks == 0); // Only the left button clicks
}

TEST_CASE("Touch: a long press is also noticed when the finger lifts", "[backend][touch]") {
    TouchFixture f;
    f.window->handleTouchPress(20.0f, 20.0f, 1s);
    f.window->handleTouchRelease(20.0f, 20.0f, 2s); // No frame in between
    CHECK(f.buttons == std::vector{MouseButton::RIGHT});
    CHECK(f.releases == 1);
    CHECK(f.clicks == 0);
}

TEST_CASE("Touch: a cancelled drag releases without clicking", "[backend][touch]") {
    const TouchFixture f;
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
    const ScrollFixture f;
    f.window->handleTouchPress(50.0f, 90.0f, 1s);
    f.window->handleTouchMove(50.0f, 60.0f, 1s + 30ms);
    f.window->handleTouchRelease(50.0f, 60.0f, 1s + 300ms);
    CHECK(f.viewport.scrollY() == -30);
    CHECK_FALSE(f.window->flinging());
}

TEST_CASE("Touch: a drag across a vertical scroller drags what is under the finger", "[backend][touch][scroll]") {
    const ScrollFixture f;
    f.window->handleTouchPress(50.0f, 50.0f, 1s);
    f.window->handleTouchMove(80.0f, 52.0f, 1s + 20ms); // Mostly horizontal: nothing scrolls that way
    CHECK(f.contentPresses == 1);
    CHECK(f.viewport.scrollY() == 0);
    f.window->handleTouchRelease(80.0f, 52.0f, 1s + 40ms);
}

namespace {
    /**
     * Two 24 x 24 targets with 10 px of hit slop, 8 px apart, in a row padded by 20 px
     */
    struct SlopFixture {
        std::shared_ptr<TestWindow> window{std::make_shared<TestWindow>()};
        FakeSystemWindow            systemWindow{window};
        Div                         &row{window->appContainer().add<Div>()};
        Div                         &a{row.add<Div>()};
        Div                         &b{row.add<Div>()};
        int                         aClicks{0};
        int                         bClicks{0};
        int                         aLeft{0};
        int                         aTop{0};

        SlopFixture() {
            row.direction(Direction::Row).padding(20).gap(8).alignItems(Align::Start);
            for (Div *target : {&a, &b}) {
                target->width(24.0f).height(24.0f).hitSlop(10);
            }
            a.onClick([this]() { ++aClicks; });
            b.onClick([this]() { ++bClicks; });
            systemWindow.layout();
            a.localToGlobal(aLeft, aTop);
        }

        /**
         * A press and release at a point relative to a's top left corner
         */
        void click(int x, int y) {
            window->handleMouseButtonChange({.x = static_cast<float>(aLeft + x), .y = static_cast<float>(aTop + y), .button = MouseButton::LEFT, .down = true});
            window->handleMouseButtonChange({.x = static_cast<float>(aLeft + x), .y = static_cast<float>(aTop + y), .button = MouseButton::LEFT, .down = false});
        }
    };
}

TEST_CASE("Hit slop: a press near a div reaches it", "[backend][touch][hitslop]") {
    SlopFixture f;
    f.click(12, 12);
    CHECK(f.aClicks == 1);
    f.click(-6, 12); // Left of it
    CHECK(f.aClicks == 2);
    f.click(12, -9); // Above it
    CHECK(f.aClicks == 3);
    f.click(-12, 12); // Too far
    f.click(12, 40);
    CHECK(f.aClicks == 3);
    CHECK(f.bClicks == 0);
}

TEST_CASE("Hit slop: between two divs, the nearest center wins", "[backend][touch][hitslop]") {
    SlopFixture f;
    // a spans 0-24, b 32-56: centers at 12 and 44
    f.click(27, 12);
    CHECK(f.aClicks == 1);
    f.click(29, 12);
    CHECK(f.bClicks == 1);
    f.click(27, 30); // Below the gap, still nearer a
    CHECK(f.aClicks == 2);
}

TEST_CASE("Hit slop: what takes presses keeps them", "[backend][touch][hitslop]") {
    SlopFixture f;
    int coverClicks = 0;
    auto &cover = f.window->appContainer().add<Div>();
    cover.position(Position::Absolute).left(0).top(0).width(static_cast<float>(f.aLeft - 2)).height(100.0f);
    cover.onClick([&]() { ++coverClicks; });
    f.systemWindow.layout();
    f.click(-4, 12); // On the cover, within a's slop
    CHECK(coverClicks == 1);
    CHECK(f.aClicks == 0);
}

TEST_CASE("Hit slop: a release away from the slop doesn't click", "[backend][touch][hitslop]") {
    const SlopFixture f;
    f.window->handleMouseButtonChange({.x = static_cast<float>(f.aLeft - 6), .y = static_cast<float>(f.aTop + 12), .button = MouseButton::LEFT, .down = true});
    f.window->handleMouseButtonChange({.x = static_cast<float>(f.aLeft - 60), .y = static_cast<float>(f.aTop + 12), .button = MouseButton::LEFT, .down = false});
    CHECK(f.aClicks == 0);
    CHECK_FALSE(f.a.getMouseDown());
}

TEST_CASE("Hit slop: a tap near a div clicks it", "[backend][touch][hitslop]") {
    using namespace std::chrono_literals;
    const SlopFixture f;
    f.window->handleTouchPress(static_cast<float>(f.aLeft + 30), static_cast<float>(f.aTop - 5), 1s);
    f.window->handleTouchRelease(static_cast<float>(f.aLeft + 30), static_cast<float>(f.aTop - 5), 1s + 50ms);
    CHECK(f.bClicks == 1);
    CHECK(f.aClicks == 0);
}

namespace {
    /**
     * Sizes `.sized` from the current density
     */
    struct SizedSheet : StyleSheet {
        void load(StyleManager *manager) override {
            manager->style(".sized").width(manager->sizes().controlSm);
        }
    };
}

TEST_CASE("Density: the style manager follows the window's input mode", "[backend][touch][theme]") {
    auto window = std::make_shared<TestWindow>();
    FakeSystemWindow systemWindow(window);
    window->theme(Theme{.metrics = {.touch = {.controlSm = 40, .hitSlop = 12}}});
    window->loadStyleSheet<SizedSheet>();
    auto &sized = window->appContainer().add<Div>().addClassName("sized");
    systemWindow.layout();
    CHECK(window->styleManager()->density() == Density::Pointer);
    CHECK(sized.getWidth() == 24);

    window->handleInputModeChange(InputMode::Touch);
    CHECK(window->styleManager()->density() == Density::Touch);
    CHECK(window->styleManager()->sizes().hitSlop == 12);
    systemWindow.layout();
    CHECK(sized.getWidth() == 40);

    // A pen is a pointer
    window->handleInputModeChange(InputMode::Pen);
    CHECK(window->styleManager()->density() == Density::Pointer);
    systemWindow.layout();
    CHECK(sized.getWidth() == 24);
}

TEST_CASE("Density: the default skins look the same in touch mode, with hit slop", "[backend][touch][theme]") {
    auto window = std::make_shared<TestWindow>();
    FakeSystemWindow systemWindow(window);
    auto &button   = window->appContainer().add<Button>("Button");
    auto &checkBox = window->appContainer().add<CheckBox>("Check");
    auto &range    = window->appContainer().add<Range<float>>();
    systemWindow.layout();
    const auto layoutOf = [&]() {
        return std::vector{
            button.getWidth(), button.getHeight(), checkBox.getWidth(), checkBox.getHeight(), range.getWidth(), range.getHeight()
        };
    };
    const auto pointerLayout = layoutOf();
    CHECK(button.computedStyle().hitSlop() == 0.0f);

    window->handleInputModeChange(InputMode::Touch);
    systemWindow.layout();
    CHECK(layoutOf() == pointerLayout);
    CHECK(button.computedStyle().hitSlop() == themes::oneDark.metrics.touch.hitSlop);
    CHECK(checkBox.computedStyle().hitSlop() == themes::oneDark.metrics.touch.hitSlop);
    CHECK(range.computedStyle().hitSlop() == themes::oneDark.metrics.touch.hitSlop);

    // Back with the next mouse motion
    window->handleInputModeChange(InputMode::Pointer);
    systemWindow.layout();
    CHECK(layoutOf() == pointerLayout);
    CHECK(button.computedStyle().hitSlop() == 0.0f);
}

TEST_CASE("Density: a new window starts with the pointer density", "[backend][touch][theme]") {
    {
        auto window = std::make_shared<TestWindow>();
        window->handleInputModeChange(InputMode::Touch);
        CHECK(window->styleManager()->density() == Density::Touch);
    }
    auto window = std::make_shared<TestWindow>();
    CHECK(window->styleManager()->density() == Density::Pointer);
}
