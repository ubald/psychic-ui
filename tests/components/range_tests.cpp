#include <chrono>
#include <format>
#include <memory>
#include <utility>
#include <vector>
#include <catch2/catch_test_macros.hpp>
#include <psychic-ui/Div.hpp>
#include <psychic-ui/components/Label.hpp>
#include <psychic-ui/components/NumberStepper.hpp>
#include <psychic-ui/components/Range.hpp>
#include <psychic-ui/psychic-ui.hpp>
#include "../backends/fake_system_window.hpp"

using namespace psychic_ui;
using namespace psychic_ui_tests;

namespace {
    /**
     * The skin's `.value` label
     */
    const Label *valueLabel(const Div &skin) {
        for (const auto &child: skin.children()) {
            if (child->classNames().contains("value")) {
                return dynamic_cast<const Label *>(child.get());
            }
        }
        return nullptr;
    }

    struct RangeFixture {
        std::shared_ptr<TestWindow> window{std::make_shared<TestWindow>()};
        FakeSystemWindow            systemWindow{window, 800, 600};
        Range<float>               &range{window->appContainer().add<Range<float>>(0.0f, 1.0f, 0.25f, nullptr).width(300)};
        std::vector<float>          changes{};
        int                         starts{0};
        int                         ends{0};

        RangeFixture() {
            range.onChange([this](float value) { changes.push_back(value); })
                 .onDragStart([this] { ++starts; })
                 .onDragEnd([this] { ++ends; });
            systemWindow.layout();
        }

        /**
         * A point on the track, in window coordinates, at this fraction of its width
         */
        [[nodiscard]] std::pair<float, float> along(const float fraction) const {
            const Div *track = nullptr;
            for (const auto &child: range.skin()->children()) {
                if (child->classNames().contains("track")) {
                    track = child.get();
                }
            }
            REQUIRE(track);
            int x = 0;
            int y = 0;
            track->localToGlobal(x, y, static_cast<int>(static_cast<float>(track->getWidth()) * fraction), track->getHeight() / 2);
            return {static_cast<float>(x), static_cast<float>(y)};
        }

        void button(const float fraction, const bool down, const MouseButton button = MouseButton::LEFT) const {
            const auto [x, y] = along(fraction);
            window->handleMouseButtonChange({.x = x, .y = y, .button = button, .down = down});
        }

        void move(const float fraction) const {
            const auto [x, y] = along(fraction);
            window->handleMouseMove({.x = x, .y = y}, false);
        }
    };
}

TEST_CASE("Range: the value part shows the value through the format", "[components][inputs][range]") {
    RangeFixture fixture;
    const Label *label = valueLabel(*fixture.range.skin());
    REQUIRE(label);
    CHECK(label->text() == "0.250"); // Three decimals by default

    fixture.range.format([](const float value) { return std::format("{:.0f} %", value * 100.0f); });
    CHECK(label->text() == "25 %"); // At once
    CHECK(fixture.range.valueString() == "25 %");

    fixture.range.value(0.5f);
    CHECK(label->text() == "50 %");

    fixture.range.format(nullptr); // Back to the default
    CHECK(label->text() == "0.500");

    // An integer range shows its integer by default
    auto &integer = fixture.window->appContainer().add<Range<int>>(0, 100, 60, nullptr);
    fixture.systemWindow.layout();
    REQUIRE(integer.skin());
    REQUIRE(valueLabel(*integer.skin()));
    CHECK(valueLabel(*integer.skin())->text() == "60");
}

TEST_CASE("Range: sync sets the value without emitting changed while the user edits emit it", "[components][inputs][range]") {
    RangeFixture fixture;
    const Label *label = valueLabel(*fixture.range.skin());
    REQUIRE(label);

    fixture.range.sync(0.75f);
    CHECK(fixture.range.value() == 0.75f);
    CHECK(label->text() == "0.750"); // The skin follows
    fixture.range.sync(2.0f);
    CHECK(fixture.range.value() == 1.0f); // Clamped
    CHECK(fixture.changes.empty());

    // A drag emits
    fixture.button(0.5f, true);
    fixture.move(0.25f);
    fixture.button(0.25f, false);
    CHECK(fixture.changes.size() == 2);

    // So does a key step
    fixture.changes.clear();
    fixture.window->requestFocus(&fixture.range, true);
    fixture.window->handleKeyPress(Key::LEFT, Mod{});
    CHECK(fixture.changes.size() == 1);
    CHECK(fixture.starts == 1); // The keys don't drag
    CHECK(fixture.ends == 1);
}

TEST_CASE("Range: a drag emits its start and its end once", "[components][inputs][range]") {
    RangeFixture fixture;

    fixture.button(0.5f, true);
    CHECK(fixture.starts == 1);
    CHECK(fixture.ends == 0);
    CHECK(fixture.changes.size() == 1); // After the start: the press's value is the drag's first
    fixture.move(0.6f);
    fixture.move(0.7f);
    CHECK(fixture.starts == 1);
    fixture.window->handleMouseButtonChange({.x = 2000, .y = 2000, .button = MouseButton::LEFT, .down = false}); // Outside
    CHECK(fixture.ends == 1);
    fixture.move(0.2f); // Over: no drag any more
    CHECK(fixture.ends == 1);
    CHECK(fixture.changes.size() == 3);

    // Another button doesn't drag
    fixture.button(0.5f, true, MouseButton::RIGHT);
    fixture.button(0.5f, false, MouseButton::RIGHT);
    CHECK(fixture.starts == 1);
    CHECK(fixture.ends == 1);

    // Disabled, nothing
    fixture.range.enabled(false);
    fixture.button(0.5f, true);
    fixture.button(0.5f, false);
    CHECK(fixture.starts == 1);
    CHECK(fixture.ends == 1);
}

TEST_CASE("Range: a touch drag emits its start and its end once", "[components][inputs][range][touch]") {
    using ms = std::chrono::milliseconds;
    const RangeFixture fixture;
    const auto [x, y] = fixture.along(0.2f);
    fixture.window->handleTouchPress(x, y, ms(1000));
    fixture.window->handleTouchMove(x + 40.0f, y, ms(1010)); // Past the slop: a press, then a drag
    fixture.window->handleTouchMove(x + 80.0f, y, ms(1020));
    CHECK(fixture.starts == 1);
    CHECK(fixture.ends == 0);
    fixture.window->handleTouchRelease(x + 80.0f, y, ms(1030));
    CHECK(fixture.starts == 1);
    CHECK(fixture.ends == 1);
    CHECK_FALSE(fixture.changes.empty());

    // A cancelled touch ends its drag too
    fixture.window->handleTouchPress(x, y, ms(2000));
    fixture.window->handleTouchMove(x + 40.0f, y, ms(2010));
    fixture.window->handleTouchCancel(ms(2020));
    CHECK(fixture.starts == 2);
    CHECK(fixture.ends == 2);
}

TEST_CASE("Range: removed mid-drag ends the drag", "[components][inputs][range]") {
    RangeFixture fixture;
    fixture.button(0.5f, true);
    CHECK(fixture.starts == 1);
    const auto keep = fixture.range.shared_from_this();
    fixture.window->appContainer().remove(fixture.range);
    CHECK(fixture.ends == 1);
}

TEST_CASE("Range<double> keeps a double's precision", "[components][inputs][range]") {
    auto                   window = std::make_shared<TestWindow>();
    const FakeSystemWindow systemWindow{window, 800, 600};
    auto                  &range  = window->appContainer().add<Range<double>>(0.0, 1.0, 0.0, nullptr);
    range.value(0.1234567890123);
    CHECK(range.value() == 0.1234567890123);
    range.sync(0.9876543210987);
    CHECK(range.value() == 0.9876543210987);
}

TEST_CASE("NumberStepper: sync sets the value without emitting changed", "[components][inputs][stepper]") {
    auto             window = std::make_shared<TestWindow>();
    FakeSystemWindow systemWindow{window, 800, 600};
    std::vector<double> changes;
    auto &stepper = window->appContainer().add<NumberStepper>(0, [&](const double value) { changes.push_back(value); });
    stepper.range(0, 10);
    systemWindow.layout();
    stepper.sync(4);
    CHECK(stepper.value() == 4);
    CHECK(stepper.text() == "4");
    stepper.sync(40);
    CHECK(stepper.value() == 10);
    CHECK(changes.empty());

    window->requestFocus(&stepper, true);
    window->handleKeyPress(Key::DOWN, Mod{});
    CHECK(changes == std::vector<double>{9});
}
