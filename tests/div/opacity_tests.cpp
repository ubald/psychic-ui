#include <cstdlib>
#include <memory>
#include <catch2/catch_test_macros.hpp>
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSurface.h"
#include <psychic-ui/Div.hpp>
#include <psychic-ui/Window.hpp>
#include <psychic-ui/style/Color.hpp>
#include <psychic-ui/style/StyleManager.hpp>

using namespace psychic_ui;

namespace {
    struct RenderWindow : Window {
        RenderWindow() : Window("opacity") {}
        using Div::render;
    };

    // A window without stylesheets, rendered over white into a raster surface
    struct Scene {
        std::shared_ptr<RenderWindow> window = std::make_shared<RenderWindow>();
        sk_sp<SkSurface>              surface = SkSurfaces::Raster(SkImageInfo::MakeN32Premul(200, 200));
        Div                           *box{nullptr};

        Scene() {
            window->setStyleManager(std::make_shared<StyleManager>());
            window->setWindowSize(200, 200);
            box = &window->appContainer().add<Div>().size(100, 100);
        }

        void render() {
            window->updateStylesAndLayout();
            surface->getCanvas()->clear(SK_ColorWHITE);
            window->render(surface->getCanvas());
        }

        // The rendered color at a point of a div, in its own coordinates
        [[nodiscard]] SkColor at(const Div &div, int x, int y) const {
            int globalX = 0;
            int globalY = 0;
            div.localToGlobal(globalX, globalY, x, y);
            SkPixmap pixmap;
            REQUIRE(surface->peekPixels(&pixmap));
            return pixmap.getColor(globalX, globalY);
        }
    };

    bool near(SkColor color, unsigned r, unsigned g, unsigned b) {
        auto close = [](unsigned a, unsigned b) { return std::abs(static_cast<int>(a) - static_cast<int>(b)) <= 2; };
        return close(SkColorGetR(color), r) && close(SkColorGetG(color), g) && close(SkColorGetB(color), b);
    }
}

TEST_CASE("opacity fades a div and its children as one group", "[div][opacity]") {
    Scene scene;
    auto &child = scene.box->backgroundColor(argb(0xFF000000)).add<Div>().size(50, 50).backgroundColor(argb(0xFFFF0000));

    SECTION("opaque by default") {
        scene.render();
        REQUIRE(near(scene.at(*scene.box, 75, 75), 0, 0, 0));
        REQUIRE(near(scene.at(child, 25, 25), 255, 0, 0));
    }

    SECTION("the child fades with its parent") {
        scene.box->opacity(0.5f);
        scene.render();
        REQUIRE(near(scene.at(*scene.box, 75, 75), 128, 128, 128));
        // Red over the black background, then the group at 50% over white
        REQUIRE(near(scene.at(child, 25, 25), 255, 128, 128));
    }

    SECTION("nested opacities multiply") {
        scene.box->backgroundColor(argb(0x00000000)).opacity(0.5f);
        child.backgroundColor(argb(0xFF000000)).opacity(0.5f);
        scene.render();
        REQUIRE(near(scene.at(child, 25, 25), 191, 191, 191));
    }

    SECTION("nothing is drawn at zero") {
        scene.box->opacity(0.0f);
        scene.render();
        REQUIRE(near(scene.at(*scene.box, 75, 75), 255, 255, 255));
        REQUIRE(near(scene.at(child, 25, 25), 255, 255, 255));
    }
}
