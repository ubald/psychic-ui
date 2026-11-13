#include <cstdlib>
#include <memory>
#include <catch2/catch_test_macros.hpp>
#include "include/core/SkCanvas.h"
#include "include/core/SkSurface.h"
#include <psychic-ui/Window.hpp>
#include <psychic-ui/components/Button.hpp>
#include "../backends/fake_system_window.hpp"

using namespace psychic_ui;

namespace {
    struct RenderWindow : Window {
        RenderWindow() : Window("paint") {}
        using Div::render;
    };

    // A window without stylesheets, rendered over white into a raster surface, with a box in its middle
    struct Scene {
        std::shared_ptr<RenderWindow> window = std::make_shared<RenderWindow>();
        sk_sp<SkSurface>              surface = SkSurfaces::Raster(SkImageInfo::MakeN32Premul(200, 200));
        Div                           *box{nullptr};

        Scene() {
            window->setStyleManager(std::make_shared<StyleManager>());
            window->setWindowSize(200, 200);
            box = &window->appContainer().add<Div>().position(Position::Absolute).left(50).top(50).size(100, 100);
        }

        void render() {
            window->updateStylesAndLayout();
            surface->getCanvas()->clear(SK_ColorWHITE);
            window->render(surface->getCanvas());
        }

        // The rendered color at a point of a div, in its own coordinates
        SkColor at(const Div &div, int x, int y) {
            int gx = 0, gy = 0;
            div.localToGlobal(gx, gy, x, y);
            SkPixmap pixmap;
            REQUIRE(surface->peekPixels(&pixmap));
            return pixmap.getColor(gx, gy);
        }
    };

    bool near(SkColor color, unsigned r, unsigned g, unsigned b) {
        auto close = [](unsigned a, unsigned b) { return std::abs(static_cast<int>(a) - static_cast<int>(b)) <= 2; };
        return close(SkColorGetR(color), r) && close(SkColorGetG(color), g) && close(SkColorGetB(color), b);
    }

    bool white(SkColor color) { return near(color, 255, 255, 255); }
}

TEST_CASE("outline and shadow properties are paint-only and not inherited", "[style][outline][shadow]") {
    Style style;
    style.outlineColor(argb(0xFFFF0000)).outlineWidth(2).outlineOffset(1)
         .shadowColor(argb(0x80000000)).shadowBlur(8).shadowOffsetX(1).shadowOffsetY(4);
    CHECK(style.outlineWidth() == 2.0f);
    CHECK(style.shadowOffsetY() == 4.0f);
    for (auto property: {Property::outlineColor, Property::outlineWidth, Property::outlineOffset, Property::shadowColor,
                         Property::shadowBlur, Property::shadowOffsetX, Property::shadowOffsetY}) {
        CHECK(paintProperties.test(index(property)));
        CHECK_FALSE(inheritedProperties.test(index(property)));
    }
    CHECK(Style{}.shadowColor().alpha() == 0); // No shadow by default
    CHECK(Style{}.outlineWidth() == 0.0f);     // Nor outline
}

TEST_CASE("an outline goes around the border box, offset from it", "[div][outline]") {
    Scene scene;
    scene.box->backgroundColor(argb(0xFF000000));

    SECTION("none by default") {
        scene.render();
        CHECK(white(scene.at(*scene.box, -2, 50)));
        CHECK(near(scene.at(*scene.box, 1, 50), 0, 0, 0));
    }

    SECTION("outside, with a gap") {
        scene.box->outlineColor(argb(0xFFFF0000)).outlineWidth(2).outlineOffset(3);
        scene.render();
        CHECK(white(scene.at(*scene.box, -2, 50)));          // The gap
        CHECK(near(scene.at(*scene.box, -4, 50), 255, 0, 0)); // The line, 3 to 5 px out
        CHECK(near(scene.at(*scene.box, 104, 50), 255, 0, 0));
        CHECK(near(scene.at(*scene.box, 50, -5), 255, 0, 0));
        CHECK(white(scene.at(*scene.box, -7, 50)));
        CHECK(near(scene.at(*scene.box, 50, 50), 0, 0, 0));  // The box itself is untouched
    }

    SECTION("inside, over the children") {
        scene.box->add<Div>().size(100, 100).backgroundColor(argb(0xFF0000FF));
        scene.box->outlineColor(argb(0xFFFF0000)).outlineWidth(2).outlineOffset(-2);
        scene.render();
        CHECK(near(scene.at(*scene.box, 1, 50), 255, 0, 0));
        CHECK(near(scene.at(*scene.box, 4, 50), 0, 0, 255));
    }

    SECTION("still drawn when only the outline is on screen") {
        scene.box->left(-110); // The box ends 10 px left of the window
        scene.box->outlineColor(argb(0xFFFF0000)).outlineWidth(6).outlineOffset(10);
        scene.render();
        SkPixmap pixmap;
        REQUIRE(scene.surface->peekPixels(&pixmap));
        CHECK(near(pixmap.getColor(2, 100), 255, 0, 0));
    }

    SECTION("faded with the div") {
        scene.box->opacity(0.5f).outlineColor(argb(0xFFFF0000)).outlineWidth(2).outlineOffset(5);
        scene.render();
        CHECK(near(scene.at(*scene.box, -6, 50), 255, 128, 128));
    }
}

TEST_CASE("a shadow goes under the border box, outside it", "[div][shadow]") {
    Scene scene;

    SECTION("none by default") {
        scene.box->backgroundColor(argb(0xFF0000FF));
        scene.render();
        CHECK(white(scene.at(*scene.box, 50, 105)));
    }

    SECTION("offset, sharp") {
        scene.box->shadowColor(argb(0xFF000000)).shadowOffsetY(10);
        scene.render();
        CHECK(near(scene.at(*scene.box, 50, 105), 0, 0, 0));
        CHECK(white(scene.at(*scene.box, 50, 112)));
        CHECK(white(scene.at(*scene.box, 50, 50))); // Not under a transparent box
    }

    SECTION("blurred") {
        scene.box->backgroundColor(argb(0xFF0000FF)).shadowColor(argb(0xFF000000)).shadowBlur(8);
        scene.render();
        const SkColor edge = scene.at(*scene.box, 50, 101);
        CHECK(SkColorGetR(edge) < 200);  // Dark next to the box
        CHECK(SkColorGetR(edge) > 0);    // But not solid
        CHECK(white(scene.at(*scene.box, 50, 120)));
        CHECK(near(scene.at(*scene.box, 50, 50), 0, 0, 255));
    }
}

TEST_CASE("a component paints its own outline, its skin doesn't repeat it", "[skins][outline]") {
    auto window = std::make_shared<psychic_ui_tests::TestWindow>();
    psychic_ui_tests::FakeSystemWindow systemWindow(window);
    auto &button = window->appContainer().add<Button>("Button");
    button.outlineColor(argb(0xFFFF0000)).outlineWidth(2).shadowColor(argb(0x80000000));
    systemWindow.layout();
    REQUIRE(button.skin());
    CHECK(button.computedStyle().outlineWidth() == 2.0f);
    CHECK(button.skin()->computedStyle().outlineWidth() == 0.0f);
    CHECK(button.skin()->computedStyle().shadowColor().alpha() == 0);
}
