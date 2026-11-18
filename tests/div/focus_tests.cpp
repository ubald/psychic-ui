// Known bugs from review-2026-11.md, tagged [!shouldfail]: remove the tag in the commit that fixes one.
#include <memory>
#include <catch2/catch_test_macros.hpp>
#include <psychic-ui/components/TextInput.hpp>
#include <psychic-ui/psychic-ui.hpp>
#include <psychic-ui/style/Values.hpp>
#include "../backends/fake_system_window.hpp"

using namespace psychic_ui;
using namespace psychic_ui_tests;

// C11: requestFocus emits focused on the whole new path even when it is the current one
TEST_CASE("C11: presses inside a focused field don't focus it again", "[div][focus][!shouldfail]") {
    auto             window = std::make_shared<TestWindow>();
    FakeSystemWindow sys{window, 800, 600};
    auto            &input = window->appContainer().add<TextInput>("hello").position(Position::Absolute).left(10).top(10).width(200);
    int              inputFocused = 0;
    int              appFocused = 0;
    input.onFocus([&] { ++inputFocused; });
    window->appContainer().onFocus([&] { ++appFocused; });
    sys.layout();
    for (int i = 0; i < 3; ++i) {
        window->handleMouseButtonChange({.x = 30, .y = 20, .button = MouseButton::LEFT, .down = true});
        window->handleMouseButtonChange({.x = 30, .y = 20, .button = MouseButton::LEFT, .down = false});
    }
    CHECK(inputFocused == 1);
    CHECK(appFocused == 1);
}

TEST_CASE("A disabled or hidden div doesn't take the focus: the path stops at its parent", "[div][focus]") {
    auto             window = std::make_shared<TestWindow>();
    FakeSystemWindow sys{window, 800, 600};
    auto            &group  = window->appContainer().add<Div>();
    auto            &field  = group.add<Div>().focusEnabled(true).size(100, 20);
    auto            &inner  = field.add<Div>().size(50, 10); // Not focusEnabled: the field takes the focus for it
    sys.layout();

    field.enabled(false);
    window->requestFocus(&inner);
    CHECK_FALSE(field.hasFocus());
    CHECK_FALSE(inner.hasFocus());
    CHECK(window->focusedDiv() == &group);

    field.enabled(true);
    window->requestFocus(&inner);
    CHECK(field.hasFocus());
    CHECK(window->focusedDiv() == &inner);

    field.visible(false);
    sys.layout();
    window->requestFocus(&field);
    CHECK_FALSE(field.hasFocus());
    CHECK(window->focusedDiv() == &group);
}
