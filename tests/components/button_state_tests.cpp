#include <memory>
#include <catch2/catch_test_macros.hpp>
#include <psychic-ui/Div.hpp>
#include <psychic-ui/Window.hpp>
#include <psychic-ui/components/Button.hpp>
#include <psychic-ui/psychic-ui.hpp>
#include <psychic-ui/skins/base/SkinState.hpp>
#include <psychic-ui/style/Color.hpp>
#include <psychic-ui/style/Properties.hpp>
#include <psychic-ui/style/StyleManager.hpp>
#include <psychic-ui/style/Values.hpp>

using namespace psychic_ui;

namespace {
    constexpr unsigned pressedColor  = 0xFFFF0000;
    constexpr unsigned selectedColor = 0xFF00FF00;
    constexpr unsigned bothColor     = 0xFF0000FF;

    // A button under rules that tell held down, toggled on and both apart, as `:active` can't
    struct Scene {
        std::shared_ptr<Window>       window  = std::make_shared<Window>("button states");
        std::shared_ptr<StyleManager> manager = std::make_shared<StyleManager>();
        Button                        *button{nullptr};
        float                         x{0};
        float                         y{0};

        Scene() {
            window->setStyleManager(manager);
            window->setWindowSize(400, 300);
            manager->style("Button:pressed").backgroundColor(argb(pressedColor));
            manager->style("Button.selected").backgroundColor(argb(selectedColor));
            manager->style("Button.selected:pressed").backgroundColor(argb(bothColor));
            manager->style("Button:active").color(argb(0xFF123456));
            button = &window->appContainer().add<Button>("Go").size(100, 40);
            window->updateStylesAndLayout();
            int globalX = 0;
            int globalY = 0;
            button->localToGlobal(globalX, globalY, 10, 10);
            x = static_cast<float>(globalX);
            y = static_cast<float>(globalY);
        }

        void press(MouseButton which = MouseButton::LEFT) const {
            window->handleMouseButtonChange({.x = x, .y = y, .button = which, .down = true});
            window->updateStylesAndLayout();
        }

        void release(MouseButton which = MouseButton::LEFT) const {
            window->handleMouseButtonChange({.x = x, .y = y, .button = which, .down = false});
            window->updateStylesAndLayout();
        }

        [[nodiscard]] unsigned background() const { return button->computedStyle().backgroundColor().argb(); }
        [[nodiscard]] bool active() const { return button->computedStyle().color().argb() == 0xFF123456; }
    };
}

TEST_CASE("a plain button is pressed while held, never selected", "[button][style]") {
    const Scene scene;
    CHECK_FALSE(scene.button->pressed());
    CHECK_FALSE(scene.button->computedStyle().has(Property::backgroundColor));

    scene.press();
    CHECK(scene.button->pressed());
    CHECK(scene.button->selected()); // As before: a plain button's selection is its press
    CHECK_FALSE(scene.button->classNames().contains("selected"));
    CHECK(scene.background() == pressedColor);
    CHECK(scene.active());
    const SkinState held = SkinState::of(scene.button);
    CHECK(held.pressed);
    CHECK_FALSE(held.selected);
    CHECK(held.active);

    scene.release();
    CHECK_FALSE(scene.button->pressed());
    CHECK_FALSE(scene.button->computedStyle().has(Property::backgroundColor));
    CHECK_FALSE(scene.active());
}

TEST_CASE("a toggle button tells held down, toggled on and both apart", "[button][style]") {
    const Scene scene;
    scene.button->toggle(true);

    SECTION("held down while off") {
        scene.press();
        CHECK(scene.background() == pressedColor);
        CHECK(scene.active());
        scene.release(); // Clicked: on
        CHECK(scene.button->selected());
        CHECK(scene.background() == selectedColor);
        CHECK(scene.active());
    }

    SECTION("on, then held down") {
        scene.button->selected(true);
        scene.window->updateStylesAndLayout();
        CHECK(scene.button->classNames().contains("selected"));
        CHECK(scene.background() == selectedColor);
        const SkinState toggledOn = SkinState::of(scene.button);
        CHECK(toggledOn.selected);
        CHECK_FALSE(toggledOn.pressed);
        CHECK(toggledOn.active);

        scene.press();
        CHECK(scene.background() == bothColor);
        const SkinState both = SkinState::of(scene.button);
        CHECK(both.selected);
        CHECK(both.pressed);

        scene.release(); // Clicked: off
        CHECK_FALSE(scene.button->selected());
        CHECK_FALSE(scene.button->classNames().contains("selected"));
        CHECK_FALSE(scene.button->computedStyle().has(Property::backgroundColor));
        CHECK_FALSE(scene.active());
    }

    SECTION("no longer a toggle") {
        scene.button->selected(true);
        scene.button->toggle(false);
        scene.window->updateStylesAndLayout();
        CHECK_FALSE(scene.button->classNames().contains("selected"));
    }
}

TEST_CASE(":pressed matches any div held down, and weighs as :active", "[style][selector]") {
    const Scene scene;
    auto       &box = scene.window->appContainer().add<Div>().size(50, 50);
    scene.manager->style("Div:pressed").opacity(0.5f);
    scene.window->updateStylesAndLayout();
    CHECK(box.computedStyle().opacity() == 1.0f);
    box.setMouseDown(true);
    scene.window->updateStylesAndLayout();
    CHECK(box.pressed());
    CHECK(box.computedStyle().opacity() == 0.5f);
    box.enabled(false);
    scene.window->updateStylesAndLayout();
    CHECK_FALSE(box.pressed());
}
