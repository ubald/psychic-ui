#include <chrono>
#include <memory>
#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include "include/core/SkCanvas.h"
#include "include/core/SkSurface.h"
#include <psychic-ui/Window.hpp>

using namespace psychic_ui;
using namespace std::chrono_literals;
using Catch::Approx;

namespace {
    struct RenderWindow : Window {
        RenderWindow() : Window("transform") {}
        using Div::render;
    };

    // A window without stylesheets, rendered over black into a raster surface, with a red box at (100, 100)
    struct Scene {
        std::shared_ptr<RenderWindow> window = std::make_shared<RenderWindow>();
        sk_sp<SkSurface>              surface = SkSurfaces::Raster(SkImageInfo::MakeN32Premul(400, 300));
        Div                           *box{nullptr};

        Scene() {
            window->setStyleManager(std::make_shared<StyleManager>());
            window->setWindowSize(400, 300);
            box = &window->appContainer().add<Div>();
            box->position(Position::Absolute).left(100).top(100).size(40, 40).backgroundColor(rgb(0xFF0000));
        }

        void render() {
            window->updateStylesAndLayout();
            surface->getCanvas()->clear(SK_ColorBLACK);
            window->render(surface->getCanvas());
        }

        bool red(int x, int y) const {
            SkPixmap pixmap;
            REQUIRE(surface->peekPixels(&pixmap));
            return pixmap.getColor(x, y) == SK_ColorRED;
        }
    };
}

TEST_CASE("translate moves what is painted, in pixels or a percent of the div", "[div][transform]") {
    Scene s{};
    s.render();
    CHECK(s.red(120, 120));

    s.box->translateX(100).translateY(50_pct); // 20 px: half its height
    s.render();
    CHECK(s.box->x() == 100); // The layout doesn't move
    CHECK(s.box->y() == 100);
    CHECK_FALSE(s.red(120, 120));
    CHECK(s.red(220, 140));
    CHECK(s.red(220, 159));
    CHECK_FALSE(s.red(220, 161));
}

TEST_CASE("scale, times scaleX and scaleY, about the transform origin", "[div][transform]") {
    Scene s{};
    s.box->scale(0.5f); // About its center, by default
    s.render();
    CHECK_FALSE(s.red(103, 103));
    CHECK(s.red(111, 111));
    CHECK(s.red(128, 128));
    CHECK_FALSE(s.red(131, 131));

    s.box->transformOriginX(0).transformOriginY(0);
    s.render();
    CHECK(s.red(103, 103));
    CHECK_FALSE(s.red(121, 121));

    s.box->scaleX(2.0f); // 1 wide, 0.5 high
    s.render();
    CHECK(s.red(138, 103));
    CHECK_FALSE(s.red(138, 121));

    s.box->scale(1.0f).scaleY(0.0f); // Nothing to paint
    s.render();
    CHECK_FALSE(s.red(110, 101));
}

TEST_CASE("Paint transforms are applied to the children too, and not to hit testing", "[div][transform]") {
    Scene s{};
    Div &child = s.box->add<Div>();
    child.size(10, 10).backgroundColor(rgb(0x00FF00));
    int presses = 0;
    s.box->onMousePress([&]() { ++presses; });
    s.box->translateX(200);
    s.render();
    SkPixmap pixmap;
    REQUIRE(s.surface->peekPixels(&pixmap));
    CHECK(pixmap.getColor(305, 105) == SK_ColorGREEN);

    // A press finds the div where the layout puts it
    const auto press = [&](float x, float y) {
        s.window->handleMouseButtonChange({.x = x, .y = y, .button = MouseButton::LEFT, .down = true});
        s.window->handleMouseButtonChange({.x = x, .y = y, .button = MouseButton::LEFT, .down = false});
    };
    press(320, 120);
    CHECK(presses == 0);
    press(120, 120);
    CHECK(presses == 1);
}

TEST_CASE("A length transitions between units through zero", "[div][transform][transition]") {
    Scene s{};
    s.box->translateY(150_pct);
    s.window->updateAnimations(0ms);
    s.render();
    s.box->translateY(0).transition(Property::translateY, Timing{100ms, Ease::Linear}); // 0 px
    s.window->updateAnimations(0ms);
    s.render();
    REQUIRE(s.box->transitioning(Property::translateY));
    s.window->updateAnimations(50ms);
    s.render();
    CHECK(s.box->computedStyle().translateY().isPercent());
    CHECK(s.box->computedStyle().translateY().value == Approx(75.0f));
    s.window->updateAnimations(100ms);
    s.render();
    CHECK_FALSE(s.box->transitioning(Property::translateY));
    CHECK(s.box->computedStyle().translateY().resolve(40) == 0.0f);

    // And back, from 0 px to a percent
    s.box->translateY(100_pct);
    s.window->updateAnimations(200ms);
    s.render();
    s.window->updateAnimations(250ms);
    s.render();
    CHECK(s.box->computedStyle().translateY().value == Approx(50.0f));
}
