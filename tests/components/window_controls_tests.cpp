#include <memory>
#include <string>
#include <vector>
#include <catch2/catch_test_macros.hpp>
#include "include/core/SkColor.h"
#include "include/core/SkPixmap.h"
#include <psychic-ui/Div.hpp>
#include <psychic-ui/components/Button.hpp>
#include <psychic-ui/components/TitleBar.hpp>
#include <psychic-ui/components/WindowControls.hpp>
#include <psychic-ui/components/WindowControlsStyle.hpp>
#include <psychic-ui/skins/pivotal/CaptionButtonSkin.hpp>
#include <psychic-ui/skins/pivotal/TitleBarButtonSkin.hpp>
#include <psychic-ui/psychic-ui.hpp>
#include <psychic-ui/style/Color.hpp>
#include <psychic-ui/themes/Theme.hpp>
#include <psychic-ui/themes/pivotal.hpp>
#include "../backends/fake_system_window.hpp"

using namespace psychic_ui;
using namespace psychic_ui_tests;

namespace {
    struct Fixture {
        std::shared_ptr<TestWindow> window{std::make_shared<TestWindow>()};
        FakeSystemWindow            systemWindow{window};
        const Theme                 &theme{themes::pivotalDark};

        void layout() { systemWindow.layout(); }

        void hover(const Div &div) {
            int x = 0;
            int y = 0;
            div.localToGlobal(x, y, div.getWidth() / 2, div.getHeight() / 2);
            window->handleMouseMove({.x = static_cast<float>(x), .y = static_cast<float>(y)}, false);
            layout();
        }

        void click(const Div &div) {
            int x = 0;
            int y = 0;
            div.localToGlobal(x, y, div.getWidth() / 2, div.getHeight() / 2);
            window->handleMouseButtonChange({.x = static_cast<float>(x), .y = static_cast<float>(y), .button = MouseButton::LEFT, .down = true});
            window->handleMouseButtonChange({.x = static_cast<float>(x), .y = static_cast<float>(y), .button = MouseButton::LEFT, .down = false});
            layout();
        }

        // The rendered color at a point of a div, in its own coordinates
        [[nodiscard]] Color at(const Div &div, int x, int y) const {
            int globalX = 0;
            int globalY = 0;
            div.localToGlobal(globalX, globalY, x, y);
            SkPixmap pixmap;
            REQUIRE(systemWindow.surface->peekPixels(&pixmap));
            return argb(pixmap.getColor(globalX, globalY));
        }

        // How many pixels of a div differ from the one at its left edge's middle (its background)
        [[nodiscard]] int marked(const Div &div) const {
            const Color background = at(div, 1, div.getHeight() / 2);
            int         count      = 0;
            for (int row = 0; row < div.getHeight(); ++row) {
                for (int column = 0; column < div.getWidth(); ++column) {
                    if (at(div, column, row) != background) {
                        ++count;
                    }
                }
            }
            return count;
        }
    };

    std::vector<const Div *> order(const WindowControls &controls) {
        std::vector<const Div *> buttons{};
        for (const auto &child: controls.children()) {
            buttons.push_back(child.get());
        }
        return buttons;
    }

    // The skin a button has now, its child 0
    const Div *skinOf(const Button &button) {
        return button.children().empty() ? nullptr : button.children().front().get();
    }

    bool light(const Color color) {
        const SkColor argb = color.argb();
        return SkColorGetR(argb) > 0xE0 && SkColorGetG(argb) > 0xE0 && SkColorGetB(argb) > 0xE0;
    }
}

TEST_CASE("window controls take the platform's style by default", "[window-controls]") {
    const Fixture fixture;
    auto    &controls = fixture.window->appContainer().add<WindowControls>();
    CHECK(controls.controlsStyle() == platformWindowControlsStyle);
    #ifdef __APPLE__
    CHECK(platformWindowControlsStyle == WindowControlsStyle::Mac);
    #else
    CHECK(platformWindowControlsStyle == WindowControlsStyle::Windows);
    #endif
    CHECK(controls.classNames().contains(std::string{windowControlsStyleClass(platformWindowControlsStyle)}));
}

TEST_CASE("macOS's window controls are close, minimize, maximize", "[window-controls]") {
    Fixture fixture;
    auto    &controls = fixture.window->appContainer().add<WindowControls>().controlsStyle(WindowControlsStyle::Mac);
    fixture.layout();
    CHECK(order(controls) == std::vector<const Div *>{&controls.closeButton(), &controls.minimizeButton(), &controls.maximizeButton()});
    CHECK(controls.classNames().contains("mac"));
    CHECK_FALSE(controls.classNames().contains("windows"));
    CHECK(dynamic_cast<const TitleBarButtonSkin *>(skinOf(controls.closeButton())) != nullptr);
    CHECK(controls.closeButton().getWidth() == 11);
}

TEST_CASE("Windows's window controls are minimize, maximize, close", "[window-controls]") {
    Fixture fixture;
    auto    &controls = fixture.window->appContainer().add<WindowControls>().controlsStyle(WindowControlsStyle::Windows);
    fixture.layout();
    CHECK(order(controls) == std::vector<const Div *>{&controls.minimizeButton(), &controls.maximizeButton(), &controls.closeButton()});
    CHECK(controls.classNames().contains("windows"));
    CHECK_FALSE(controls.classNames().contains("mac"));
    for (const Button *button: {&controls.minimizeButton(), &controls.maximizeButton(), &controls.closeButton()}) {
        CHECK(dynamic_cast<const CaptionButtonSkin *>(skinOf(*button)) != nullptr);
        CHECK(button->getWidth() == 28);
        CHECK(button->getHeight() == 24);
    }

    // And back
    controls.controlsStyle(WindowControlsStyle::Mac);
    fixture.layout();
    CHECK(order(controls) == std::vector<const Div *>{&controls.closeButton(), &controls.minimizeButton(), &controls.maximizeButton()});
    CHECK(controls.classNames().contains("mac"));
    CHECK_FALSE(controls.classNames().contains("windows"));
}

TEST_CASE("Windows's window controls draw a glyph each, close turning the danger color under the pointer", "[window-controls]") {
    Fixture fixture;
    auto    &controls = fixture.window->appContainer().add<WindowControls>().controlsStyle(WindowControlsStyle::Windows);
    fixture.layout();
    const Palette &palette = fixture.theme.palette;

    for (const Button *button: {&controls.minimizeButton(), &controls.maximizeButton(), &controls.closeButton()}) {
        CHECK(fixture.marked(*button) > 6);
    }
    // A glyph, not a fill: the corners show what is under the button
    const Color under = fixture.at(controls.closeButton(), 1, 12);
    CHECK(under != palette.danger);

    fixture.hover(controls.closeButton());
    const Button &close = controls.closeButton();
    CHECK(fixture.at(close, 3, 12) == palette.danger);
    CHECK(light(fixture.at(close, close.getWidth() / 2, close.getHeight() / 2))); // Where the × crosses

    // Another one is raised, subtly, and close is back to nothing
    fixture.hover(controls.minimizeButton());
    CHECK(fixture.at(close, 3, 12) == under);
    CHECK(fixture.at(controls.minimizeButton(), 3, 12) != under);
    CHECK(fixture.at(controls.minimizeButton(), 3, 12) != palette.danger);
}

TEST_CASE("a title bar puts its window controls where the platform does", "[window-controls]") {
    Fixture fixture;
    auto    &bar = fixture.window->appContainer().add<TitleBar>();

    bar.controls().controlsStyle(WindowControlsStyle::Mac);
    REQUIRE_FALSE(bar.children().empty());
    CHECK(bar.children().front().get() == &bar.controls());

    bar.controls().controlsStyle(WindowControlsStyle::Windows);
    CHECK(bar.children().back().get() == &bar.controls());
    fixture.layout();
    CHECK(bar.controls().x() + bar.controls().getWidth() > bar.getWidth() - 20);

    bar.controls().controlsStyle(WindowControlsStyle::Mac);
    CHECK(bar.children().front().get() == &bar.controls());
}

TEST_CASE("Windows's maximize button maximizes the window and then shows restore", "[window-controls]") {
    Fixture fixture;
    auto    &controls = fixture.window->appContainer().add<WindowControls>().controlsStyle(WindowControlsStyle::Windows);
    fixture.layout();
    const Button &maximize = controls.maximizeButton();

    // The glyph's pixels, row by row
    const auto glyph = [&]() {
        std::vector<Color> pixels{};
        for (int row = 0; row < maximize.getHeight(); ++row) {
            for (int column = 0; column < maximize.getWidth(); ++column) {
                pixels.push_back(fixture.at(maximize, column, row));
            }
        }
        return pixels;
    };
    const auto square = glyph();

    fixture.click(maximize);
    CHECK(fixture.window->maximized());
    CHECK_FALSE(fixture.window->getFullscreen());
    const auto restore = glyph();
    CHECK(restore != square);
    // The 10 px glyph's square is (9, 7) to (19, 17) in the 28 × 24 button. Restore's back square, 2 px up and right,
    // shows its left edge above the front square, which starts 2 px lower; the square's top left corner is gone
    const Color background = fixture.at(maximize, 1, 12);
    CHECK(fixture.at(maximize, 11, 8) != background);  // The back square's left edge, above the front one
    CHECK(fixture.at(maximize, 12, 9) != background);  // The front square's top edge
    CHECK(fixture.at(maximize, 9, 7) == background);   // Where the unmaximized square's corner was
    CHECK(square[(8 * maximize.getWidth()) + 11] == background); // The square has nothing there

    fixture.click(maximize);
    CHECK_FALSE(fixture.window->maximized());
    CHECK(glyph() == square);
}

TEST_CASE("macOS's maximize button goes full screen", "[window-controls]") {
    Fixture fixture;
    auto    &controls = fixture.window->appContainer().add<WindowControls>().controlsStyle(WindowControlsStyle::Mac);
    fixture.layout();
    fixture.click(controls.maximizeButton());
    CHECK(fixture.window->getFullscreen());
    CHECK_FALSE(fixture.window->maximized());
}
