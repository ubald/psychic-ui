#include <array>
#include <cmath>
#include <cstddef>
#include <vector>
#include <catch2/catch_message.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkRRect.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSurface.h"
#include <psychic-ui/paint/Crisp.hpp>
#include <psychic-ui/style/Color.hpp>

using namespace psychic_ui;
using Catch::Matchers::WithinAbs;

namespace {
    constexpr int surfaceSize = 64;

    // A transparent raster surface whose canvas is scaled by the pixel ratio, as a window's is
    sk_sp<SkSurface> surfaceAt(const float ratio) {
        auto surface = SkSurfaces::Raster(SkImageInfo::MakeN32Premul(surfaceSize, surfaceSize));
        surface->getCanvas()->clear(SK_ColorTRANSPARENT);
        surface->getCanvas()->scale(ratio, ratio);
        return surface;
    }

    SkPixmap pixels(SkSurface *surface) {
        SkPixmap pixmap;
        REQUIRE(surface->peekPixels(&pixmap));
        return pixmap;
    }

    std::vector<SkColor> row(SkSurface *surface, const int down) {
        const SkPixmap       pixmap = pixels(surface);
        std::vector<SkColor> colors{};
        colors.reserve(static_cast<std::size_t>(pixmap.width()));
        for (int across = 0; across < pixmap.width(); ++across) {
            colors.push_back(pixmap.getColor(across, down));
        }
        return colors;
    }

    std::vector<SkColor> column(SkSurface *surface, const int across) {
        const SkPixmap       pixmap = pixels(surface);
        std::vector<SkColor> colors{};
        colors.reserve(static_cast<std::size_t>(pixmap.height()));
        for (int down = 0; down < pixmap.height(); ++down) {
            colors.push_back(pixmap.getColor(across, down));
        }
        return colors;
    }

    // Every alpha is 0 or 255; the length of the first opaque run
    int firstOpaqueRun(const std::vector<SkColor> &colors) {
        int  run   = 0;
        bool ended = false;
        for (const SkColor color: colors) {
            const auto alpha = SkColorGetA(color);
            INFO("alpha " << static_cast<int>(alpha));
            REQUIRE((alpha == 0 || alpha == 255));
            if (alpha == 255 && !ended) {
                ++run;
            } else if (alpha == 0 && run > 0) {
                ended = true;
            }
        }
        return run;
    }

    const Color white = rgb(0xffffff);
    const Color red   = rgb(0xff0000);
    const Color green = rgb(0x00ff00);
    const Color blue  = rgb(0x0000ff);
}

TEST_CASE("a crisp width is whole device pixels", "[paint]") {
    for (const auto &[ratio, expected]: std::array{std::array{1.0f, 1.0f}, std::array{1.5f, 2.0f}, std::array{2.0f, 2.0f}}) {
        INFO("ratio " << ratio);
        auto surface = surfaceAt(ratio);
        REQUIRE_THAT(paint::crispWidth(surface->getCanvas(), 1) * ratio, WithinAbs(expected, 1e-4));
    }
    auto one = surfaceAt(1);
    REQUIRE_THAT(paint::crispWidth(one->getCanvas(), 0.25f), WithinAbs(1, 1e-4));
    auto oneAndAHalf = surfaceAt(1.5f);
    REQUIRE_THAT(paint::crispWidth(oneAndAHalf->getCanvas(), 2) * 1.5f, WithinAbs(3, 1e-4));
}

TEST_CASE("crisp rings land on device pixels", "[paint]") {
    const float  ratio = GENERATE(1.0f, 1.5f, 2.0f);
    const SkRect rect  = GENERATE(SkRect::MakeXYWH(4, 4, 24, 16), SkRect::MakeXYWH(3.3f, 7.7f, 24, 16));
    INFO("ratio " << ratio << ", origin " << rect.x() << ", " << rect.y());

    auto       surface = surfaceAt(ratio);
    SkCanvas  *canvas  = surface->getCanvas();
    const auto shape   = SkRRect::MakeRectXY(rect, 4.5f, 4.5f);
    const std::array ring{paint::Ring{.color = white, .width = 1}};
    paint::rings(canvas, shape, ring);

    const auto expected = static_cast<int>(std::lround(paint::crispWidth(canvas, 1) * ratio));
    const auto middleY  = static_cast<int>(std::floor(rect.centerY() * ratio));
    const auto middleX  = static_cast<int>(std::floor(rect.centerX() * ratio));
    REQUIRE(firstOpaqueRun(row(surface.get(), middleY)) == expected);
    REQUIRE(firstOpaqueRun(column(surface.get(), middleX)) == expected);
}

TEST_CASE("nested rings touch without gaps", "[paint]") {
    auto      surface = surfaceAt(1.5f);
    SkCanvas *canvas  = surface->getCanvas();
    const auto shape  = SkRRect::MakeRectXY(SkRect::MakeXYWH(4, 4, 24, 16), 4.5f, 4.5f);
    const std::array rings{paint::Ring{.color = red, .width = 1}, paint::Ring{.color = green, .width = 1}};
    paint::rings(canvas, shape, rings, blue);

    // The shape is 6 to 42 device pixels across, 6 to 30 down
    const auto colors = row(surface.get(), 18);
    for (int across = 0; across < 6; ++across) {
        INFO("x " << across);
        REQUIRE(colors[across] == SK_ColorTRANSPARENT);
    }
    REQUIRE(colors[6] == SK_ColorRED);
    REQUIRE(colors[7] == SK_ColorRED);
    REQUIRE(colors[8] == SK_ColorGREEN);
    REQUIRE(colors[9] == SK_ColorGREEN);
    for (int across = 10; across < 38; ++across) {
        INFO("x " << across);
        REQUIRE(colors[across] == SK_ColorBLUE);
    }
    REQUIRE(colors[38] == SK_ColorGREEN);
    REQUIRE(colors[39] == SK_ColorGREEN);
    REQUIRE(colors[40] == SK_ColorRED);
    REQUIRE(colors[41] == SK_ColorRED);
    for (int across = 42; across < surfaceSize; ++across) {
        INFO("x " << across);
        REQUIRE(colors[across] == SK_ColorTRANSPARENT);
    }
}

TEST_CASE("rings stop when the shape is used up", "[paint]") {
    auto      surface = surfaceAt(1);
    SkCanvas *canvas  = surface->getCanvas();
    const auto shape  = SkRRect::MakeRect(SkRect::MakeXYWH(10, 10, 6, 6));
    const std::array rings{
        paint::Ring{.color = red, .width = 2}, paint::Ring{.color = green, .width = 2},
        paint::Ring{.color = white, .width = 2}
    };
    const SkRRect left = paint::rings(canvas, shape, rings, blue);
    REQUIRE(left.isEmpty());

    const SkPixmap pixmap = pixels(surface.get());
    for (int down = 0; down < pixmap.height(); ++down) {
        for (int across = 0; across < pixmap.width(); ++across) {
            const SkColor color  = pixmap.getColor(across, down);
            const bool    inside = across >= 10 && across < 16 && down >= 10 && down < 16;
            INFO("pixel " << across << ", " << down);
            if (!inside) {
                REQUIRE(color == SK_ColorTRANSPARENT);
            }
            REQUIRE(color != SK_ColorBLUE);
        }
    }
}

TEST_CASE("a transformed canvas draws unsnapped", "[paint]") {
    auto      surface = surfaceAt(1);
    SkCanvas *canvas  = surface->getCanvas();
    const auto shape  = SkRRect::MakeRectXY(SkRect::MakeXYWH(20.3f, 20.7f, 24, 16), 4.5f, 4.5f);

    SECTION("rotated") {
        canvas->rotate(10);
    }
    SECTION("scaled unevenly") {
        canvas->scale(1.5f, 2);
    }
    REQUIRE(paint::snap(canvas, shape) == shape);
    REQUIRE(paint::crispWidth(canvas, 1) == 1.0f);

    const std::array ring{paint::Ring{.color = white, .width = 1}};
    paint::rings(canvas, shape, ring);
    const SkPixmap pixmap = pixels(surface.get());
    bool           drawn  = false;
    for (int down = 0; down < pixmap.height() && !drawn; ++down) {
        for (int across = 0; across < pixmap.width() && !drawn; ++across) {
            drawn = SkColorGetA(pixmap.getColor(across, down)) > 0;
        }
    }
    REQUIRE(drawn);
}
