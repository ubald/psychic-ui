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
