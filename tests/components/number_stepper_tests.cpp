// Known bugs from docs/review-2026-11.md, tagged [!shouldfail]: remove the tag in the commit that fixes one.
#include <catch2/catch_test_macros.hpp>
#include <psychic-ui/components/NumberStepper.hpp>
#include <psychic-ui/components/TextInput.hpp>
#include "../backends/fake_system_window.hpp"

using namespace psychic_ui;
using namespace psychic_ui_tests;

// C13: the typed value is kept unrounded while the field shows it rounded
TEST_CASE("C13: NumberStepper keeps the value it shows, rounded to its decimals", "[components][inputs][stepper][!shouldfail]") {
    auto             window = std::make_shared<TestWindow>();
    FakeSystemWindow sys{window, 800, 600};
    double           last    = 0;
    auto            &stepper = window->appContainer().add<NumberStepper>(0, [&](double v) { last = v; });
    stepper.decimals(0);
    sys.layout();
    window->requestFocus(&stepper, true);
    stepper.input()->text("1.75");
    window->handleKeyPress(Key::ENTER, Mod{});
    sys.layout();
    CHECK(stepper.input()->text() == "2");
    CHECK(stepper.value() == 2);
    CHECK(last == 2);
}

// C14: a step finer than the decimals rounds back to where it started
TEST_CASE("C14: NumberStepper steps down with a step finer than its decimals", "[components][inputs][stepper][!shouldfail]") {
    auto             window = std::make_shared<TestWindow>();
    FakeSystemWindow sys{window, 800, 600};
    auto            &stepper = window->appContainer().add<NumberStepper>(1);
    stepper.step(0.5).decimals(0);
    sys.layout();
    window->requestFocus(&stepper, true);
    window->handleKeyPress(Key::DOWN, Mod{});
    CHECK(stepper.value() < 1);
}
