#include <cstdlib>
#include <memory>
#include <catch2/catch_test_macros.hpp>
#include "include/core/SkCanvas.h"
#include "include/core/SkSurface.h"
#include <psychic-ui/Window.hpp>
#include <psychic-ui/components/Label.hpp>
#include <psychic-ui/components/Text.hpp>

using namespace psychic_ui;

namespace {
    struct RenderWindow : Window {
        RenderWindow() : Window("text style") {}
        using Div::render;
    };

    // A window without stylesheets, rendered over white into a raster surface
    struct Scene {
        std::shared_ptr<RenderWindow> window  = std::make_shared<RenderWindow>();
        sk_sp<SkSurface>              surface = SkSurfaces::Raster(SkImageInfo::MakeN32Premul(200, 100));

        Scene() {
            window->setStyleManager(std::make_shared<StyleManager>());
            window->setWindowSize(200, 100);
        }

        void render() {
            window->updateStylesAndLayout();
            surface->getCanvas()->clear(SK_ColorWHITE);
            window->render(surface->getCanvas());
        }

        SkColor at(const Div &div, int x, int y) {
            int gx = 0, gy = 0;
            div.localToGlobal(gx, gy, x, y);
            SkPixmap pixmap;
            REQUIRE(surface->peekPixels(&pixmap));
            return pixmap.getColor(gx, gy);
        }
    };
}

TEST_CASE("Font smoothing is an inherited style property, grayscale by default", "[text][style]") {
    CHECK(Style{}.fontSmoothing() == FontSmoothing::Grayscale);
    CHECK(inheritedProperties.test(static_cast<size_t>(Property::fontSmoothing)));

    Scene s;
    auto &box   = s.window->appContainer().add<Div>();
    auto &label = box.add<Label>("Text");
    s.render();
    CHECK(label.computedStyle().fontSmoothing() == FontSmoothing::Grayscale);

    box.fontSmoothing(FontSmoothing::Subpixel);
    s.render();
    CHECK(label.computedStyle().fontSmoothing() == FontSmoothing::Subpixel);

    label.fontSmoothing(FontSmoothing::None);
    s.render();
    CHECK(label.computedStyle().fontSmoothing() == FontSmoothing::None);
}

TEST_CASE("Selection and caret colors are inherited, so a field's parts take its own", "[text][style]") {
    for (auto property: {Property::selectionColor, Property::selectionBackgroundColor, Property::caretColor}) {
        CHECK(inheritedProperties.test(static_cast<size_t>(property)));
        CHECK(animatableProperties.test(static_cast<size_t>(property)));
    }

    Scene s;
    auto &box  = s.window->appContainer().add<Div>();
    auto &text = box.add<Text>("Text");
    box.selectionColor(rgb(0x112233)).selectionBackgroundColor(rgb(0x445566)).caretColor(rgb(0x778899));
    s.render();
    CHECK(text.computedStyle().selectionColor() == rgb(0x112233));
    CHECK(text.computedStyle().selectionBackgroundColor() == rgb(0x445566));
    CHECK(text.computedStyle().caretColor() == rgb(0x778899));
}

TEST_CASE("The caret is painted in caretColor, else in the text's color", "[text][style]") {
    Scene s;
    auto &text = s.window->appContainer().add<Text>("");
    text.editable(true)
        .color(rgb(0x0000FF))
        .position(Position::Absolute)
        .left(20)
        .top(20)
        .width(100);
    s.render();
    s.window->requestFocus(&text);
    s.render();
    const int middle = static_cast<int>(text.getHeight() / 2.0f);
    CHECK(s.at(text, 0, middle) == SkColorSetRGB(0x00, 0x00, 0xFF));

    text.caretColor(rgb(0xFF0000));
    s.render();
    CHECK(s.at(text, 0, middle) == SkColorSetRGB(0xFF, 0x00, 0x00));
}
