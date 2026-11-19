#include <chrono>
#include <cmath>
#include <cstddef>
#include <memory>
#include <optional>
#include <string>
#include <vector>
#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <psychic-ui/DragAndDrop.hpp>
#include <psychic-ui/DragEvent.hpp>
#include <psychic-ui/DragPayload.hpp>
#include <psychic-ui/DragPreview.hpp>
#include <psychic-ui/Overlay.hpp>
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
        std::vector<ZoomEvent>   zooms{};

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
            window->onZoom([this](const ZoomEvent &event) { zooms.push_back(event); });
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
    TouchFixture fixture;
    fixture.window->handleTouchPress(20.5f, 30.5f, 1s);
    CHECK(fixture.window->inputMode() == InputMode::Touch);
    CHECK(fixture.presses == 0); // Could still become a drag, a pan or a long press
    fixture.window->handleTouchMove(23.0f, 32.0f, 1s + 50ms); // Within the slop
    fixture.window->handleTouchRelease(23.0f, 32.0f, 1s + 100ms);
    CHECK(fixture.presses == 1);
    int originX = 0;
    int originY = 0;
    fixture.target.localToGlobal(originX, originY);
    CHECK(fixture.pressX == 20 - originX);
    CHECK(fixture.pressY == 30 - originY);
    CHECK(fixture.releases == 1);
    CHECK(fixture.clicks == 1);
    CHECK(fixture.buttons == std::vector{MouseButton::LEFT});
    CHECK_FALSE(fixture.target.mouseOver()); // No hover left behind
}

TEST_CASE("Touch: a drag starts past the slop, from where the finger landed", "[backend][touch]") {
    const TouchFixture fixture;
    fixture.window->handleTouchPress(20.0f, 20.0f, 1s);
    fixture.window->handleTouchMove(25.0f, 20.0f, 1s + 10ms);
    CHECK(fixture.presses == 0);
    const int movesBefore = fixture.moves;
    fixture.window->handleTouchMove(40.0f, 20.0f, 1s + 20ms);
    CHECK(fixture.presses == 1);
    CHECK(fixture.pressX < 20); // Where it landed, in the div
    CHECK(fixture.moves > movesBefore);
    fixture.window->handleTouchMove(150.0f, 20.0f, 1s + 30ms); // Outside: the drag goes on
    fixture.window->handleTouchRelease(150.0f, 20.0f, 1s + 40ms);
    CHECK(fixture.releases == 1);
    CHECK(fixture.clicks == 0); // Released outside
}

TEST_CASE("Touch: a long press is a right button press", "[backend][touch]") {
    TouchFixture fixture;
    fixture.window->handleTouchPress(20.0f, 20.0f, 1s);
    fixture.window->updateGestures(1s + 499ms);
    CHECK(fixture.presses == 0);
    fixture.window->updateGestures(1s + 500ms);
    CHECK(fixture.presses == 1);
    CHECK(fixture.buttons == std::vector{MouseButton::RIGHT});
    CHECK(fixture.releases == 0); // Until the finger lifts
    fixture.window->handleTouchRelease(20.0f, 20.0f, 2s);
    CHECK(fixture.releases == 1);
    CHECK(fixture.buttons == std::vector{MouseButton::RIGHT}); // No tap after it
    CHECK(fixture.clicks == 0); // Only the left button clicks
}

TEST_CASE("Touch: a long press is also noticed when the finger lifts", "[backend][touch]") {
    TouchFixture fixture;
    fixture.window->handleTouchPress(20.0f, 20.0f, 1s);
    fixture.window->handleTouchRelease(20.0f, 20.0f, 2s); // No frame in between
    CHECK(fixture.buttons == std::vector{MouseButton::RIGHT});
    CHECK(fixture.releases == 1);
    CHECK(fixture.clicks == 0);
}

TEST_CASE("Touch: a cancelled drag releases without clicking", "[backend][touch]") {
    const TouchFixture fixture;
    fixture.window->handleTouchPress(20.0f, 20.0f, 1s);
    fixture.window->handleTouchMove(40.0f, 20.0f, 1s + 10ms);
    fixture.window->handleTouchMove(30.0f, 20.0f, 1s + 20ms);
    fixture.window->handleTouchCancel(1s + 30ms);
    CHECK(fixture.presses == 1);
    CHECK(fixture.releases == 1);
    CHECK(fixture.clicks == 0);
}

TEST_CASE("Touch: a finger drag pans, kinetic scrolling decelerates and a touch stops it", "[backend][touch][scroll]") {
    ScrollFixture fixture;
    REQUIRE(fixture.viewport.contentHeight() == 1000);

    fixture.swipeUp();
    CHECK(fixture.viewport.scrollY() == -60); // The content followed the finger
    CHECK(fixture.contentPresses == 0);       // A pan presses nothing
    REQUIRE(fixture.window->flinging());

    // Every frame moves less than the one before
    std::vector<int> steps{};
    int              last = fixture.viewport.scrollY();
    for (int frame = 0; frame < 5; ++frame) {
        fixture.time += 16ms;
        fixture.window->updateGestures(fixture.time);
        steps.push_back(last - fixture.viewport.scrollY());
        last = fixture.viewport.scrollY();
    }
    CHECK(steps.front() > 0);
    CHECK(steps.front() <= 16); // About 1000 px/s
    CHECK(steps.back() < steps.front());
    for (std::size_t i = 1; i < steps.size(); ++i) {
        CHECK(steps[i] <= steps[i - 1]);
    }

    // A finger stops it, and isn't a tap
    fixture.window->handleTouchPress(50.0f, 50.0f, fixture.time);
    CHECK_FALSE(fixture.window->flinging());
    const int stopped = fixture.viewport.scrollY();
    fixture.time += 16ms;
    fixture.window->updateGestures(fixture.time);
    fixture.window->handleTouchRelease(50.0f, 50.0f, fixture.time);
    CHECK(fixture.viewport.scrollY() == stopped);
    CHECK(fixture.contentPresses == 0);
}

TEST_CASE("Touch: kinetic scrolling stops by itself", "[backend][touch][scroll]") {
    ScrollFixture fixture(100000.0f);
    fixture.swipeUp();
    REQUIRE(fixture.window->flinging());
    int frames = 0;
    while (fixture.window->flinging() && frames < 1000) {
        fixture.time += 16ms;
        fixture.window->updateGestures(fixture.time);
        ++frames;
    }
    CHECK_FALSE(fixture.window->flinging());
    CHECK(frames < 300); // A few seconds
    // About v0 / rate (1000 px/s over -ln(0.998) * 1000 per second: 500 px), after the 60 px panned
    CHECK(fixture.viewport.scrollY() < -400);
    CHECK(fixture.viewport.scrollY() > -600);
}

TEST_CASE("Touch: kinetic scrolling stops at the end of the content", "[backend][touch][scroll]") {
    ScrollFixture fixture(200.0f);
    fixture.swipeUp();
    REQUIRE(fixture.window->flinging());
    for (int frame = 0; frame < 20 && fixture.window->flinging(); ++frame) {
        fixture.time += 16ms;
        fixture.window->updateGestures(fixture.time);
    }
    CHECK_FALSE(fixture.window->flinging());
    CHECK(fixture.viewport.scrollY() == -100);
}

TEST_CASE("Touch: a pan that rests before lifting doesn't fling", "[backend][touch][scroll]") {
    const ScrollFixture fixture;
    fixture.window->handleTouchPress(50.0f, 90.0f, 1s);
    fixture.window->handleTouchMove(50.0f, 60.0f, 1s + 30ms);
    fixture.window->handleTouchRelease(50.0f, 60.0f, 1s + 300ms);
    CHECK(fixture.viewport.scrollY() == -30);
    CHECK_FALSE(fixture.window->flinging());
}

TEST_CASE("Touch: a drag across a vertical scroller drags what is under the finger", "[backend][touch][scroll]") {
    const ScrollFixture fixture;
    fixture.window->handleTouchPress(50.0f, 50.0f, 1s);
    fixture.window->handleTouchMove(80.0f, 52.0f, 1s + 20ms); // Mostly horizontal: nothing scrolls that way
    CHECK(fixture.contentPresses == 1);
    CHECK(fixture.viewport.scrollY() == 0);
    fixture.window->handleTouchRelease(80.0f, 52.0f, 1s + 40ms);
}

TEST_CASE("Touch: two fingers spreading zoom in around their center", "[backend][touch][zoom]") {
    TouchFixture fixture;
    fixture.window->handleTouchPress(100.0f, 50.0f, 1s, 1);
    fixture.window->handleTouchPress(200.0f, 50.0f, 1s + 10ms, 2);
    fixture.window->updateGestures(1s + 10ms);
    CHECK(fixture.zooms.empty());

    // Both fingers' moves, then a frame: one zoom, twice as close around 150
    fixture.window->handleTouchMove(50.0f, 50.0f, 1s + 20ms, 1);
    fixture.window->handleTouchMove(250.0f, 50.0f, 1s + 20ms, 2);
    CHECK(fixture.zooms.empty());
    fixture.window->updateGestures(1s + 25ms);
    REQUIRE(fixture.zooms.size() == 1);
    const ZoomEvent &zoom = fixture.zooms.front();
    CHECK(zoom.scale == Catch::Approx(2.0f));
    CHECK(zoom.windowX == Catch::Approx(150.0f));
    CHECK(zoom.windowY == Catch::Approx(50.0f));
    CHECK(zoom.panX == Catch::Approx(0.0f));
    CHECK(zoom.panY == Catch::Approx(0.0f));
    CHECK(zoom.source == InputMode::Touch);
    fixture.window->updateGestures(1s + 30ms);
    CHECK(fixture.zooms.size() == 1); // Nothing moved since

    // A finger lifting elsewhere than it last moved zooms at once
    fixture.window->handleTouchRelease(0.0f, 50.0f, 1s + 40ms, 1);
    REQUIRE(fixture.zooms.size() == 2);
    CHECK(fixture.zooms[1].scale == Catch::Approx(1.25f));
    fixture.window->handleTouchRelease(250.0f, 50.0f, 1s + 50ms, 2);
    CHECK(fixture.zooms.size() == 2);
    CHECK(fixture.presses == 0);
    CHECK(fixture.clicks == 0);
}

TEST_CASE("Touch: two fingers moving together pan", "[backend][touch][zoom]") {
    TouchFixture fixture;
    fixture.window->handleTouchPress(100.0f, 50.0f, 1s, 1);
    fixture.window->handleTouchPress(200.0f, 50.0f, 1s + 10ms, 2);
    fixture.window->handleTouchMove(130.0f, 50.0f, 1s + 20ms, 1);
    fixture.window->handleTouchMove(230.0f, 50.0f, 1s + 20ms, 2);
    fixture.window->updateGestures(1s + 25ms);
    REQUIRE(fixture.zooms.size() == 1);
    const ZoomEvent &zoom = fixture.zooms.front();
    CHECK(zoom.scale == Catch::Approx(1.0f));
    CHECK(zoom.panX == Catch::Approx(30.0f));
    CHECK(zoom.panY == Catch::Approx(0.0f));
    CHECK(zoom.windowX == Catch::Approx(180.0f));
}

TEST_CASE("Touch: a second finger turns a pan into a pinch", "[backend][touch][scroll][zoom]") {
    ScrollFixture          fixture;
    std::vector<ZoomEvent> zooms{};
    fixture.viewport.onZoom([&](const ZoomEvent &event) { zooms.push_back(event); });

    // A fast pan, which would fling if lifted
    fixture.window->handleTouchPress(50.0f, 90.0f, 1s, 1);
    fixture.window->handleTouchMove(50.0f, 70.0f, 1s + 10ms, 1);
    fixture.window->handleTouchMove(50.0f, 50.0f, 1s + 20ms, 1);
    REQUIRE(fixture.viewport.scrollY() == -40);

    fixture.window->handleTouchPress(50.0f, 10.0f, 1s + 25ms, 2);
    fixture.window->handleTouchMove(50.0f, 70.0f, 1s + 30ms, 1);
    fixture.window->handleTouchMove(50.0f, 5.0f, 1s + 30ms, 2);
    fixture.window->updateGestures(1s + 32ms);
    CHECK(fixture.viewport.scrollY() == -40); // The pan stopped
    REQUIRE(zooms.size() == 1);
    CHECK(zooms.front().source == InputMode::Touch);
    CHECK(zooms.front().scale == Catch::Approx(65.0f / 40.0f));

    fixture.window->handleTouchRelease(50.0f, 70.0f, 1s + 35ms, 1);
    fixture.window->handleTouchRelease(50.0f, 5.0f, 1s + 35ms, 2);
    CHECK_FALSE(fixture.window->flinging());
    CHECK(fixture.contentPresses == 0);
}

TEST_CASE("Touch: a second finger releases a drag without clicking", "[backend][touch][zoom]") {
    TouchFixture fixture;
    fixture.window->handleTouchPress(20.0f, 20.0f, 1s, 1);
    fixture.window->handleTouchMove(40.0f, 20.0f, 1s + 10ms, 1);
    REQUIRE(fixture.presses == 1);
    fixture.window->handleTouchPress(60.0f, 60.0f, 1s + 20ms, 2);
    CHECK(fixture.releases == 1);
    CHECK(fixture.clicks == 0);
    const int movesBefore = fixture.moves;
    fixture.window->handleTouchMove(45.0f, 20.0f, 1s + 30ms, 1);
    fixture.window->updateGestures(1s + 35ms);
    CHECK(fixture.moves == movesBefore); // Pinching, no longer dragging
    CHECK(fixture.zooms.size() == 1);
    fixture.window->handleTouchRelease(45.0f, 20.0f, 1s + 40ms, 1);
    fixture.window->handleTouchRelease(60.0f, 60.0f, 1s + 40ms, 2);
    CHECK(fixture.presses == 1);
    CHECK(fixture.releases == 1);
    CHECK(fixture.clicks == 0);
}

TEST_CASE("Touch: lifting one finger of a pinch doesn't tap", "[backend][touch][zoom]") {
    TouchFixture fixture;
    fixture.window->handleTouchPress(20.0f, 20.0f, 1s, 1);
    fixture.window->handleTouchPress(60.0f, 60.0f, 1s + 10ms, 2);
    fixture.window->handleTouchRelease(20.0f, 20.0f, 1s + 50ms, 1);

    // The other finger does nothing more: no zoom, no tap, no drag, no long press
    const int movesBefore = fixture.moves;
    fixture.window->handleTouchMove(90.0f, 60.0f, 1s + 60ms, 2);
    fixture.window->updateGestures(3s);
    CHECK(fixture.moves == movesBefore);
    fixture.window->handleTouchRelease(90.0f, 60.0f, 3s, 2);
    CHECK(fixture.zooms.empty());
    CHECK(fixture.presses == 0);
    CHECK(fixture.clicks == 0);

    // The next finger taps again
    fixture.window->handleTouchPress(20.0f, 20.0f, 4s, 3);
    fixture.window->handleTouchRelease(20.0f, 20.0f, 4s + 50ms, 3);
    CHECK(fixture.clicks == 1);
}

TEST_CASE("Touch: a third finger is ignored", "[backend][touch][zoom]") {
    TouchFixture fixture;
    fixture.window->handleTouchPress(100.0f, 50.0f, 1s, 1);
    fixture.window->handleTouchPress(200.0f, 50.0f, 1s + 10ms, 2);
    fixture.window->handleTouchPress(150.0f, 90.0f, 1s + 20ms, 3);
    fixture.window->handleTouchMove(150.0f, 10.0f, 1s + 30ms, 3);
    fixture.window->updateGestures(1s + 35ms);
    CHECK(fixture.zooms.empty());
    fixture.window->handleTouchRelease(150.0f, 10.0f, 1s + 40ms, 3);

    // The pinch goes on
    fixture.window->handleTouchMove(0.0f, 50.0f, 1s + 50ms, 1);
    fixture.window->updateGestures(1s + 55ms);
    REQUIRE(fixture.zooms.size() == 1);
    CHECK(fixture.zooms[0].scale == Catch::Approx(2.0f));
    CHECK(fixture.zooms[0].windowX == Catch::Approx(100.0f));
    CHECK(fixture.presses == 0);
}

namespace {
    /**
     * A 100 x 100 scrollable list at (0, 0) of ten 40 px drag sources, and a 100 x 100 drop target at (200, 0) that
     * takes their payloads. The drag signals, the sources' clicks and context requests are logged.
     */
    struct TouchDragFixture {
        std::shared_ptr<TestWindow> window{std::make_shared<TestWindow>()};
        FakeSystemWindow            systemWindow{window};
        Div                         &list{window->appContainer().add<Div>()};
        Div                         &target{window->appContainer().add<Div>()};
        std::vector<Div *>          items{};
        std::vector<std::string>    log{};
        int                         sourceCalls{0};
        int                         clicks{0};
        int                         contextRequests{0};

        TouchDragFixture() {
            list.position(Position::Absolute).left(0).top(0).width(100).height(100).overflow(Overflow::Scroll);
            for (int i = 0; i < 10; ++i) {
                auto &item = list.add<Div>().height(40).shrink(0);
                item.dragSource([this, i](const MouseEvent &) -> std::optional<DragPayload> {
                    ++sourceCalls;
                    return DragPayload{.type = "test/chip", .label = "item" + std::to_string(i)};
                });
                item.onClick([this] { ++clicks; });
                item.onContextMenuRequest([this] { ++contextRequests; });
                item.onDragStart([this, i] { log.push_back("item" + std::to_string(i) + " start"); });
                item.onDragEnd([this, i](const DragEvent &event) {
                    log.push_back("item" + std::to_string(i) + " end" + (event.accepted ? " accepted" : ""));
                });
                items.push_back(&item);
            }
            target.position(Position::Absolute).left(200).top(0).width(100).height(100)
                  .dropTarget([](const DragPayload &payload) { return payload.type == "test/chip"; })
                  .onDragEnter([this] { log.emplace_back("target enter"); })
                  .onDragExit([this] { log.emplace_back("target exit"); })
                  .onDrop([this](const DragEvent &event) {
                      log.emplace_back("target drop");
                      CHECK(event.source == InputMode::Touch);
                  });
            systemWindow.layout();
        }

        /**
         * A finger held on (50, 20), the first item, until it's a long press
         */
        void longPress() const {
            window->handleTouchPress(50.0f, 20.0f, 1s);
            window->updateGestures(1s + 500ms);
        }
    };
}

TEST_CASE("a long press on a source starts a touch drag", "[backend][touch][drag]") {
    TouchDragFixture fixture;
    fixture.window->handleTouchPress(50.0f, 90.0f, 1s);
    fixture.window->updateGestures(1s + 499ms);
    CHECK_FALSE(fixture.window->dragAndDrop().active());
    CHECK(fixture.sourceCalls == 0);

    fixture.window->updateGestures(1s + 500ms);
    REQUIRE(fixture.window->dragAndDrop().active());
    CHECK(fixture.window->dragAndDrop().inputMode() == InputMode::Touch);
    CHECK(fixture.window->dragAndDrop().source().get() == fixture.items[2]);
    CHECK(fixture.items[2]->classNames().contains("dragging"));
    CHECK(fixture.log == std::vector<std::string>{"item2 start"});
    CHECK(fixture.contextRequests == 0); // The drag takes the long press

    // The preview shows above the finger, centered on it
    fixture.systemWindow.layout();
    const DragPreview *preview = fixture.window->dragPreview();
    REQUIRE(preview != nullptr);
    int overlayX = 0;
    int overlayY = 0;
    fixture.window->overlay().localToGlobal(overlayX, overlayY);
    CHECK(static_cast<float>(overlayY + preview->y() + preview->getHeight()) <= 90.0f - DragPreview::fingerOffset);
    CHECK(std::abs(static_cast<float>(overlayX + preview->x()) + (static_cast<float>(preview->getWidth()) / 2.0f) - 50.0f) <= 1.0f);

    // The finger drives it, and drops where it lifts
    fixture.window->handleTouchMove(150.0f, 40.0f, 1s + 600ms);
    fixture.window->handleTouchMove(250.0f, 50.0f, 1s + 700ms);
    CHECK(fixture.log == std::vector<std::string>{"item2 start", "target enter"});
    CHECK(fixture.target.classNames().contains("drag-over"));
    CHECK(fixture.list.scrollY() == 0);
    fixture.window->handleTouchRelease(250.0f, 50.0f, 1s + 800ms);
    CHECK_FALSE(fixture.window->dragAndDrop().active());
    CHECK(fixture.log == std::vector<std::string>{"item2 start", "target enter", "target drop", "item2 end accepted"});
    CHECK(fixture.clicks == 0);
    CHECK(fixture.contextRequests == 0);
    fixture.systemWindow.layout();
    CHECK(fixture.window->dragPreview() == nullptr);
}

TEST_CASE("a touch swipe on a source pans its scroller; a long press drags", "[backend][touch][drag][scroll]") {
    const TouchDragFixture fixture;
    fixture.window->handleTouchPress(50.0f, 90.0f, 1s);
    fixture.window->handleTouchMove(50.0f, 70.0f, 1s + 10ms);
    fixture.window->handleTouchMove(50.0f, 50.0f, 1s + 20ms);
    fixture.window->updateGestures(1s + 600ms); // Panning: no long press
    fixture.window->handleTouchRelease(50.0f, 50.0f, 1s + 600ms);
    CHECK(fixture.list.scrollY() < 0);
    CHECK_FALSE(fixture.window->dragAndDrop().active());
    CHECK(fixture.sourceCalls == 0);
    CHECK(fixture.log.empty());

    // A swipe across, which nothing scrolls along, presses and moves, without a drag
    fixture.window->handleTouchPress(50.0f, 50.0f, 3s);
    fixture.window->handleTouchMove(90.0f, 52.0f, 3s + 20ms);
    fixture.window->handleTouchMove(250.0f, 52.0f, 3s + 40ms);
    CHECK_FALSE(fixture.window->dragAndDrop().active());
    fixture.window->handleTouchRelease(250.0f, 52.0f, 3s + 60ms);
    CHECK(fixture.sourceCalls == 0);
    CHECK(fixture.log.empty());

    // Held, it drags what is under the finger
    fixture.window->handleTouchPress(50.0f, 50.0f, 5s);
    fixture.window->updateGestures(5s + 500ms);
    REQUIRE(fixture.window->dragAndDrop().active());
    int itemX = 0;
    int itemY = 0;
    const Div *source = fixture.window->dragAndDrop().source().get();
    REQUIRE(source != nullptr);
    source->localToGlobal(itemX, itemY);
    CHECK(static_cast<float>(itemY) <= 50.0f);
    CHECK(static_cast<float>(itemY + source->getHeight()) > 50.0f);
    fixture.window->handleTouchRelease(50.0f, 50.0f, 5s + 600ms);
    CHECK_FALSE(fixture.window->dragAndDrop().active());
}

TEST_CASE("a touch cancel ends the drag without a drop", "[backend][touch][drag]") {
    TouchDragFixture fixture;
    fixture.longPress();
    REQUIRE(fixture.window->dragAndDrop().active());
    fixture.window->handleTouchMove(250.0f, 50.0f, 1s + 600ms);
    CHECK(fixture.log == std::vector<std::string>{"item0 start", "target enter"});

    fixture.window->handleTouchCancel(1s + 700ms);
    CHECK_FALSE(fixture.window->dragAndDrop().active());
    CHECK(fixture.log == std::vector<std::string>{"item0 start", "target enter", "target exit", "item0 end"});
    CHECK_FALSE(fixture.target.classNames().contains("drag-over"));
    CHECK_FALSE(fixture.items[0]->classNames().contains("dragging"));
    fixture.systemWindow.layout();
    CHECK(fixture.window->dragPreview() == nullptr);

    // The finger lifting after does nothing more
    fixture.window->handleTouchRelease(250.0f, 50.0f, 1s + 800ms);
    CHECK(fixture.log.size() == 4);
    CHECK(fixture.clicks == 0);

    // A second finger neither pinches nor ends a drag: the first one's lifting does, outside the target
    fixture.log.clear();
    fixture.longPress();
    REQUIRE(fixture.window->dragAndDrop().active());
    fixture.window->handleTouchPress(80.0f, 80.0f, 1s + 600ms, 1);
    fixture.window->handleTouchRelease(80.0f, 80.0f, 1s + 700ms, 1);
    CHECK(fixture.window->dragAndDrop().active());
    fixture.window->handleTouchRelease(50.0f, 20.0f, 1s + 800ms);
    CHECK(fixture.log == std::vector<std::string>{"item0 start", "item0 end"});
}

TEST_CASE("a long press on a source that refuses is the context press", "[backend][touch][drag]") {
    auto             window = std::make_shared<TestWindow>();
    FakeSystemWindow systemWindow(window);
    int              sourceCalls     = 0;
    int              contextRequests = 0;
    window->appContainer().add<Div>().width(100).height(40)
          .dragSource([&sourceCalls](const MouseEvent &) -> std::optional<DragPayload> {
              ++sourceCalls;
              return std::nullopt;
          })
          .onContextMenuRequest([&contextRequests] { ++contextRequests; });
    systemWindow.layout();

    window->handleTouchPress(50.0f, 20.0f, 1s);
    window->updateGestures(1s + 500ms);
    CHECK(sourceCalls == 1);
    CHECK(contextRequests == 1);
    CHECK_FALSE(window->dragAndDrop().active());
    CHECK(window->dragAndDrop().state() == DragAndDrop::State::Idle);
    window->handleTouchRelease(50.0f, 20.0f, 1s + 600ms);
    CHECK(sourceCalls == 1);
}

TEST_CASE("a long press on a control inside a source is the context press", "[backend][touch][drag]") {
    auto             window = std::make_shared<TestWindow>();
    FakeSystemWindow systemWindow(window);
    int              sourceCalls     = 0;
    int              contextRequests = 0;
    auto &row = window->appContainer().add<Div>().direction(Direction::Row).width(300).height(40);
    row.add<Div>().width(150).height(40);
    auto &range = row.add<Range<float>>(0.0f, 1.0f, 0.5f, nullptr).width(150);
    row.dragSource([&sourceCalls](const MouseEvent &) -> std::optional<DragPayload> {
           ++sourceCalls;
           return DragPayload{.type = "test/row", .label = "row"};
       })
       .onContextMenuRequest([&contextRequests] { ++contextRequests; });
    systemWindow.layout();
    int x = 0;
    int y = 0;
    range.localToGlobal(x, y, range.getWidth() / 2, range.getHeight() / 2);

    window->handleTouchPress(static_cast<float>(x), static_cast<float>(y), 1s);
    window->updateGestures(1s + 500ms);
    CHECK(sourceCalls == 0);
    CHECK(contextRequests == 1);
    CHECK_FALSE(window->dragAndDrop().active());
    window->handleTouchRelease(static_cast<float>(x), static_cast<float>(y), 1s + 600ms);

    // On the row's plain part, it drags
    window->handleTouchPress(20.0f, 20.0f, 3s);
    window->updateGestures(3s + 500ms);
    CHECK(sourceCalls == 1);
    CHECK(window->dragAndDrop().active());
    window->handleTouchRelease(20.0f, 20.0f, 3s + 600ms);
}

TEST_CASE("a source keeps a long press from a neighbour's hit slop", "[backend][touch][drag]") {
    auto             window = std::make_shared<TestWindow>();
    FakeSystemWindow systemWindow(window);
    int              sourceCalls       = 0;
    int              neighbourRequests = 0;
    window->appContainer().add<Div>().position(Position::Absolute).left(0).top(0).width(100).height(40)
          .dragSource([&sourceCalls](const MouseEvent &) -> std::optional<DragPayload> {
              ++sourceCalls;
              return DragPayload{.type = "test/chip", .label = "chip"};
          });
    window->appContainer().add<Div>().position(Position::Absolute).left(105).top(0).width(24).height(24).hitSlop(10)
          .onContextMenuRequest([&neighbourRequests] { ++neighbourRequests; });
    systemWindow.layout();

    window->handleTouchPress(97.0f, 10.0f, 1s); // On the source, within the neighbour's slop
    window->updateGestures(1s + 500ms);
    CHECK(sourceCalls == 1);
    CHECK(window->dragAndDrop().active());
    CHECK(neighbourRequests == 0);
    window->handleTouchRelease(97.0f, 10.0f, 1s + 600ms);
}

TEST_CASE("a cancelled touch drag leaves the next mouse release alone", "[backend][touch][drag]") {
    TouchDragFixture fixture;
    std::vector<bool> changes{};
    fixture.items[0]->onMouseButtonChange([&changes](const MouseEvent &event) { changes.push_back(event.down); });
    fixture.longPress();
    REQUIRE(fixture.window->dragAndDrop().active());
    fixture.window->handleTouchCancel(1s + 600ms);
    CHECK_FALSE(fixture.window->dragAndDrop().active());

    // A release with no press first (a mouse button held from before) reaches the divs
    fixture.window->handleMouseButtonChange({.x = 50, .y = 20, .button = MouseButton::LEFT, .down = false});
    CHECK(changes == std::vector<bool>{false});
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
    SlopFixture fixture;
    fixture.click(12, 12);
    CHECK(fixture.aClicks == 1);
    fixture.click(-6, 12); // Left of it
    CHECK(fixture.aClicks == 2);
    fixture.click(12, -9); // Above it
    CHECK(fixture.aClicks == 3);
    fixture.click(-12, 12); // Too far
    fixture.click(12, 40);
    CHECK(fixture.aClicks == 3);
    CHECK(fixture.bClicks == 0);
}

TEST_CASE("Hit slop: between two divs, the nearest center wins", "[backend][touch][hitslop]") {
    SlopFixture fixture;
    // a spans 0-24, b 32-56: centers at 12 and 44
    fixture.click(27, 12);
    CHECK(fixture.aClicks == 1);
    fixture.click(29, 12);
    CHECK(fixture.bClicks == 1);
    fixture.click(27, 30); // Below the gap, still nearer a
    CHECK(fixture.aClicks == 2);
}

TEST_CASE("Hit slop: what takes presses keeps them", "[backend][touch][hitslop]") {
    SlopFixture fixture;
    int coverClicks = 0;
    auto &cover = fixture.window->appContainer().add<Div>();
    cover.position(Position::Absolute).left(0).top(0).width(static_cast<float>(fixture.aLeft - 2)).height(100.0f);
    cover.onClick([&]() { ++coverClicks; });
    fixture.systemWindow.layout();
    fixture.click(-4, 12); // On the cover, within a's slop
    CHECK(coverClicks == 1);
    CHECK(fixture.aClicks == 0);
}

TEST_CASE("Hit slop: a release away from the slop doesn't click", "[backend][touch][hitslop]") {
    const SlopFixture fixture;
    fixture.window->handleMouseButtonChange({.x = static_cast<float>(fixture.aLeft - 6), .y = static_cast<float>(fixture.aTop + 12), .button = MouseButton::LEFT, .down = true});
    fixture.window->handleMouseButtonChange({.x = static_cast<float>(fixture.aLeft - 60), .y = static_cast<float>(fixture.aTop + 12), .button = MouseButton::LEFT, .down = false});
    CHECK(fixture.aClicks == 0);
    CHECK_FALSE(fixture.a.getMouseDown());
}

TEST_CASE("Hit slop: a tap near a div clicks it", "[backend][touch][hitslop]") {
    using namespace std::chrono_literals;
    const SlopFixture fixture;
    fixture.window->handleTouchPress(static_cast<float>(fixture.aLeft + 30), static_cast<float>(fixture.aTop - 5), 1s);
    fixture.window->handleTouchRelease(static_cast<float>(fixture.aLeft + 30), static_cast<float>(fixture.aTop - 5), 1s + 50ms);
    CHECK(fixture.bClicks == 1);
    CHECK(fixture.aClicks == 0);
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
