#include <cstdlib>
#include <memory>
#include <catch2/catch_test_macros.hpp>
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSurface.h"
#include <psychic-ui/Div.hpp>
#include <psychic-ui/Window.hpp>
#include <psychic-ui/style/Color.hpp>
#include <psychic-ui/style/StyleManager.hpp>
#include <psychic-ui/style/Values.hpp>

using namespace psychic_ui;

namespace {
    struct RenderWindow : Window {
        RenderWindow() : Window("borders") {}
        using Div::render;
    };

    // A window without stylesheets, rendered over white into a raster surface, with a 100 px box in its middle
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

        // The rendered color at a point of the box, in its own coordinates
        [[nodiscard]] SkColor at(int x, int y) const {
            int globalX = 0;
            int globalY = 0;
            box->localToGlobal(globalX, globalY, x, y);
            SkPixmap pixmap;
            REQUIRE(surface->peekPixels(&pixmap));
            return pixmap.getColor(globalX, globalY);
        }
    };

    bool near(SkColor color, unsigned r, unsigned g, unsigned b) {
        auto close = [](unsigned a, unsigned b) { return std::abs(static_cast<int>(a) - static_cast<int>(b)) <= 2; };
        return close(SkColorGetR(color), r) && close(SkColorGetG(color), g) && close(SkColorGetB(color), b);
    }

    bool white(SkColor color) { return near(color, 255, 255, 255); }
    bool red(SkColor color) { return near(color, 255, 0, 0); }
    bool green(SkColor color) { return near(color, 0, 255, 0); }
    bool blue(SkColor color) { return near(color, 0, 0, 255); }
}

TEST_CASE("a rounded div draws only the sides that have a border", "[div][border]") {
    Scene scene;
    scene.box->borderRadius(20).borderColor(argb(0xFFFF0000));

    SECTION("one side, without a background") {
        scene.box->borderLeft(4);
        scene.render();
        CHECK(red(scene.at(1, 50)));
        CHECK(red(scene.at(3, 50)));
        CHECK(white(scene.at(5, 50)));
        CHECK(white(scene.at(99, 50))); // No hairline on the other sides
        CHECK(white(scene.at(98, 50)));
        CHECK(white(scene.at(50, 0)));
        CHECK(white(scene.at(50, 99)));
    }

    SECTION("one side, over a background that reaches the other edges") {
        scene.box->borderRight(4).backgroundColor(argb(0xFF0000FF));
        scene.render();
        CHECK(red(scene.at(98, 50)));
        CHECK(blue(scene.at(94, 50)));
        CHECK(blue(scene.at(0, 50)));
        CHECK(blue(scene.at(50, 0)));
        CHECK(blue(scene.at(50, 99)));
    }

    SECTION("the side thins out along the rounded corner, as CSS's does") {
        scene.box->borderLeft(4);
        scene.render();
        // At 10 px from the top, the outer curve is 2.7 px in and the inner one (16 by 20) 6.1 px in
        CHECK(white(scene.at(1, 10)));
        CHECK(red(scene.at(4, 10)));
        CHECK(white(scene.at(8, 10)));
        CHECK(white(scene.at(10, 1))); // Nothing along the top
    }

    SECTION("equal sides set one by one") {
        scene.box->borderHorizontal(3).borderVertical(3);
        scene.render();
        CHECK(red(scene.at(1, 50)));
        CHECK(red(scene.at(98, 50)));
        CHECK(red(scene.at(50, 1)));
        CHECK(red(scene.at(50, 98)));
        CHECK(white(scene.at(50, 50)));
    }

    SECTION("sides of their own colors") {
        scene.box->borderLeft(4).borderTop(4).borderLeftColor(argb(0xFF00FF00));
        scene.render();
        CHECK(green(scene.at(1, 50)));
        CHECK(red(scene.at(50, 1)));
        CHECK(white(scene.at(98, 50)));
        CHECK(white(scene.at(50, 98)));
    }
}

TEST_CASE("a rounded div doesn't draw a side without a color", "[div][border]") {
    Scene scene;
    scene.box->borderRadius(20).borderLeft(4).borderLeftColor(argb(0xFF00FF00)).borderRight(4);
    scene.render();
    CHECK(green(scene.at(1, 50)));
    CHECK(white(scene.at(97, 50)));
    CHECK(white(scene.at(99, 50)));
}
