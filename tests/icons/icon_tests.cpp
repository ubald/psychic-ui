#include <array>
#include <cstddef>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <catch2/catch_message.hpp>
#include <catch2/catch_test_macros.hpp>
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSurface.h"
#include <psychic-ui/Div.hpp>
#include <psychic-ui/Window.hpp>
#include <psychic-ui/components/Icon.hpp>
#include <psychic-ui/icons/IconRegistry.hpp>
#include <psychic-ui/icons/IconSet.hpp>
#include <psychic-ui/paint/Icon.hpp>
#include <psychic-ui/style/Color.hpp>
#include <psychic-ui/style/StyleManager.hpp>
#include <psychic-ui/style/Values.hpp>

using namespace psychic_ui;

namespace {
    // What std::cerr receives while it lives
    struct CaptureErrors {
        std::ostringstream stream;
        std::streambuf     *previous{std::cerr.rdbuf(stream.rdbuf())};

        CaptureErrors() = default;
        CaptureErrors(const CaptureErrors &) = delete;
        CaptureErrors &operator=(const CaptureErrors &) = delete;
        CaptureErrors(CaptureErrors &&) = delete;
        CaptureErrors &operator=(CaptureErrors &&) = delete;

        ~CaptureErrors() { std::cerr.rdbuf(previous); }

        std::string text() const { return stream.str(); }
    };

    std::size_t count(const std::string &text, const std::string &part) {
        std::size_t n = 0;
        for (auto at = text.find(part); at != std::string::npos; at = text.find(part, at + part.size())) {
            ++n;
        }
        return n;
    }

    // Where a raster surface has ink (anything not white), as a bounding box; empty if none
    SkIRect inkBounds(SkSurface *surface) {
        SkPixmap pixmap;
        REQUIRE(surface->peekPixels(&pixmap));
        SkIRect bounds = SkIRect::MakeEmpty();
        for (int row = 0; row < pixmap.height(); ++row) {
            for (int column = 0; column < pixmap.width(); ++column) {
                if (pixmap.getColor(column, row) != SK_ColorWHITE) {
                    bounds.join(SkIRect::MakeXYWH(column, row, 1, 1));
                }
            }
        }
        return bounds;
    }

    sk_sp<SkSurface> whiteSurface(int width, int height) {
        auto surface = SkSurfaces::Raster(SkImageInfo::MakeN32Premul(width, height));
        surface->getCanvas()->clear(SK_ColorWHITE);
        return surface;
    }

    constexpr std::array squareElements{IconElement{.paint = IconPaint::Fill, .d = "M2 2h6v6H2z"}};
    constexpr IconDefinition square{.name = "test-square", .grid = 10.0f, .stroke = 1.0f, .elements = squareElements};
}

TEST_CASE("the shared registry has the bundled icons", "[icons]") {
    auto &registry = IconRegistry::shared();

    // No name in both sets
    REQUIRE(registry.size() == icons::pivotalIcons.size() + icons::lucideIcons.size());
    REQUIRE(registry.has("check"));
    REQUIRE(registry.has("chevron-down"));
    REQUIRE(registry.has("star-filled"));
    REQUIRE(registry.has("chevron-up"));
    REQUIRE(registry.has("eye-off"));
    REQUIRE_FALSE(registry.has("no-such-icon"));
    // Pivotal's set comes first, in its order, then Lucide's
    auto names = registry.names();
    REQUIRE(names.front() == icons::pivotalIcons.front().name);
    REQUIRE(names[icons::pivotalIcons.size()] == icons::lucideIcons.front().name);
    REQUIRE(registry.shape("chevron-up")->grid == 24.0f);
    REQUIRE(registry.shape("chevron-down")->grid == 16.0f);
}

TEST_CASE("every bundled icon parses to paths inside its grid", "[icons]") {
    auto &registry = IconRegistry::shared();
    for (auto name: registry.names()) {
        INFO(name);
        const IconShape *shape = registry.shape(name);
        REQUIRE(shape);
        REQUIRE_FALSE(shape->paths.empty());
        for (const auto &[paint, path]: shape->paths) {
            REQUIRE_FALSE(path.isEmpty());
            const SkRect bounds = path.computeTightBounds();
            REQUIRE(bounds.left() >= 0.0f);
            REQUIRE(bounds.top() >= 0.0f);
            REQUIRE(bounds.right() <= shape->grid);
            REQUIRE(bounds.bottom() <= shape->grid);
        }
    }
}

TEST_CASE("a shape is parsed once and kept", "[icons]") {
    IconRegistry registry{icons::pivotalIcons};
    const IconShape *first = registry.shape("search");
    REQUIRE(first);
    REQUIRE(registry.shape("search") == first);
}

TEST_CASE("an application adds its own icons, or replaces one", "[icons]") {
    IconRegistry registry{};
    REQUIRE(registry.size() == 0);

    registry.add(square);
    REQUIRE(registry.has("test-square"));
    REQUIRE(registry.shape("test-square")->grid == 10.0f);

    constexpr std::array bigger{IconElement{.paint = IconPaint::Stroke, .d = "M1 1h8v8H1z"}};
    registry.add(IconDefinition{.name = "test-square", .grid = 12.0f, .stroke = 1.0f, .elements = bigger});
    REQUIRE(registry.size() == 1);
    const IconShape *shape = registry.shape("test-square");
    REQUIRE(shape->grid == 12.0f);
    REQUIRE(shape->paths.front().first == IconPaint::Stroke);
}

TEST_CASE("an unknown icon draws nothing and is reported once", "[icons]") {
    IconRegistry registry{icons::pivotalIcons};
    auto surface = whiteSurface(32, 32);
    const CaptureErrors errors;

    REQUIRE(registry.shape("no-such-icon") == nullptr);
    REQUIRE_FALSE(paint::icon(surface->getCanvas(), "no-such-icon", SkRect::MakeWH(32, 32), argb(0xFF000000), paint::iconStroke, &registry));
    REQUIRE_FALSE(paint::icon(surface->getCanvas(), "no-such-icon", SkRect::MakeWH(32, 32), argb(0xFF000000), paint::iconStroke, &registry));

    REQUIRE(inkBounds(surface.get()).isEmpty());
    REQUIRE(count(errors.text(), "\"no-such-icon\"") == 1);
}

TEST_CASE("path data that doesn't parse draws nothing and is reported once", "[icons]") {
    constexpr std::array broken{IconElement{.paint = IconPaint::Stroke, .d = "M1 1 Q"}};
    IconRegistry registry{};
    registry.add(IconDefinition{.name = "broken", .grid = 16.0f, .stroke = 1.3f, .elements = broken});
    const CaptureErrors errors;

    REQUIRE(registry.shape("broken") == nullptr);
    REQUIRE(registry.shape("broken") == nullptr);
    REQUIRE(count(errors.text(), "\"broken\"") == 1);
}

TEST_CASE("an icon is drawn inside its box at any size", "[icons]") {
    for (const float size: {14.0f, 16.0f, 20.0f}) {
        for (const float scale: {1.0f, 2.0f}) {
            for (const auto *name: {"search", "check", "settings", "more", "star-filled", "chevron-up", "circle-alert", "grip-vertical"}) {
                INFO(name << " at " << size << " px, scale " << scale);
                auto surface = whiteSurface(64, 64);
                SkCanvas *canvas = surface->getCanvas();
                canvas->scale(scale, scale);
                const SkRect box = SkRect::MakeXYWH(5, 7, size, size);
                REQUIRE(paint::icon(canvas, name, box, argb(0xFF000000)));

                const SkIRect ink = inkBounds(surface.get());
                REQUIRE_FALSE(ink.isEmpty());
                const SkIRect device = SkIRect::MakeLTRB(
                    static_cast<int>(box.left() * scale), static_cast<int>(box.top() * scale),
                    static_cast<int>(box.right() * scale), static_cast<int>(box.bottom() * scale)
                );
                REQUIRE(device.contains(ink));
            }
        }
    }
}

TEST_CASE("an icon's box is put on the pixel grid", "[icons]") {
    // Half a pixel off: drawn as at the next whole pixel
    auto aligned = whiteSurface(32, 32);
    auto offset  = whiteSurface(32, 32);
    paint::icon(aligned->getCanvas(), "check", SkRect::MakeXYWH(8, 8, 16, 16), argb(0xFF000000));
    paint::icon(offset->getCanvas(), "check", SkRect::MakeXYWH(7.6f, 7.6f, 16, 16), argb(0xFF000000));

    SkPixmap a;
    SkPixmap b;
    REQUIRE(aligned->peekPixels(&a));
    REQUIRE(offset->peekPixels(&b));
    for (int row = 0; row < 32; ++row) {
        for (int column = 0; column < 32; ++column) {
            REQUIRE(a.getColor(column, row) == b.getColor(column, row));
        }
    }
}

namespace {
    struct RenderWindow : Window {
        RenderWindow() : Window("icons") {}
        using Div::render;
    };
}

TEST_CASE("Icon measures its size and draws inside its box", "[icons]") {
    auto window = std::make_shared<RenderWindow>();
    window->setStyleManager(std::make_shared<StyleManager>());
    window->setWindowSize(100, 100);
    auto &row = window->appContainer().add<Div>().row().alignItems(Align::Start).padding(10);
    auto &icon = row.add<Icon>("search").size(20).color(argb(0xFF000000));
    auto &plain = row.add<Icon>("x").padding(2);

    window->updateStylesAndLayout();
    REQUIRE(icon.name() == "search");
    REQUIRE(icon.getWidth() == 20.0f);
    REQUIRE(icon.getHeight() == 20.0f);
    // The default size, Pivotal's medium, plus the padding
    REQUIRE(plain.getWidth() == 14.0f + 4.0f);

    auto surface = whiteSurface(100, 100);
    window->render(surface->getCanvas());
    const SkIRect ink = inkBounds(surface.get());
    REQUIRE_FALSE(ink.isEmpty());
    int x = 0;
    int y = 0;
    icon.localToGlobal(x, y, 0, 0);
    // Both icons draw inside their own boxes, together inside the row of them
    REQUIRE(SkIRect::MakeXYWH(x, y, 20 + 18, 20).contains(ink));

    SECTION("a new size is measured again") {
        icon.size(16);
        window->updateStylesAndLayout();
        REQUIRE(icon.getWidth() == 16.0f);
    }

    SECTION("an unknown name draws nothing") {
        const CaptureErrors errors;
        icon.name("no-such-icon-either");
        plain.name("");
        auto blank = whiteSurface(100, 100);
        window->render(blank->getCanvas());
        REQUIRE(inkBounds(blank.get()).isEmpty());
    }
}
