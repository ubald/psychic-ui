#include <memory>
#include <cmath>
#include <string>
#include <filesystem>
#include <catch2/catch_test_macros.hpp>
#include "include/core/SkCanvas.h"
#include "include/core/SkFont.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTypeface.h"
#include <psychic-ui/style/StyleManager.hpp>
#include <psychic-ui/themes/Theme.hpp>
#include <psychic-ui/utils/ExecutablePath.hpp>
#include <psychic-ui/text/TextLayout.hpp>

using namespace psychic_ui;

namespace {
    // The theme's font paths are relative to the resource root (the executable's directory): load from the sources instead
    std::string resource(std::string_view themePath) {
        constexpr std::string_view prefix = "res/";
        REQUIRE(themePath.starts_with(prefix));
        return std::string{PSYCHIC_UI_TEST_RES} + std::string{themePath.substr(prefix.size())};
    }

    std::shared_ptr<StyleManager> withInter() {
        auto manager = std::make_shared<StyleManager>();
        const Typography type{};
        for (const Font &font: {type.regular, type.medium, type.semiBold}) {
            manager->loadFont(std::string{font.name}, resource(font.path));
        }
        return manager;
    }

    // Laid out as Text does: subpixel positioning, so slight hinting
    TextLayout layout(const std::shared_ptr<StyleManager> &manager, const std::string &family, float size) {
        TextLayout text{};
        text.setText(icu::UnicodeString::fromUTF8("The quick brown fox jumps over the lazy dog"));
        text.setStyle(
            TextLayoutStyle{manager->fontCollection(), family, size, size * 1.5f, 0.0f, FontSmoothing::Grayscale, TextAlign::Start}
        );
        return text;
    }
}

TEST_CASE("Inter is bundled and loads by family name", "[text][fonts]") {
    auto manager = withInter();

    for (auto [family, weight]: {std::pair{"Inter", 400}, {"Inter Medium", 500}, {"Inter SemiBold", 600}}) {
        INFO(family);
        sk_sp<SkTypeface> typeface = manager->font(family);
        REQUIRE(typeface);
        SkString name;
        typeface->getFamilyName(&name);
        REQUIRE(std::string{name.c_str()} == "Inter");
        REQUIRE(typeface->fontStyle().weight() == weight);
    }
}

TEST_CASE("text lays out in Inter at small sizes, slightly hinted", "[text][fonts]") {
    auto manager = withInter();

    for (float size: {11.0f, 12.0f, 13.0f, 14.0f}) {
        INFO(size << " px");
        auto regular  = layout(manager, "Inter", size);
        auto medium   = layout(manager, "Inter Medium", size);
        auto fallback = layout(manager, "No Such Family", size);
        float width = regular.maxContentWidth();
        REQUIRE(width > 0.0f);
        // Inter itself, not the system's fallback; medium is a little wider than regular
        REQUIRE(width != fallback.maxContentWidth());
        REQUIRE(medium.maxContentWidth() > width);
        // Linear advances: the width scales with the size, not rounded per glyph
        REQUIRE(std::abs(width / size - layout(manager, "Inter", 100.0f).maxContentWidth() / 100.0f) < 0.02f);

        auto surface = SkSurfaces::Raster(SkImageInfo::MakeN32Premul(400, 40));
        surface->getCanvas()->clear(SK_ColorWHITE);
        regular.layout(400.0f);
        regular.paint(surface->getCanvas(), 0.0f, 0.0f, SK_ColorBLACK);
        SkPixmap pixmap;
        REQUIRE(surface->peekPixels(&pixmap));
        bool ink = false;
        for (int y = 0; y < pixmap.height() && !ink; ++y) {
            for (int x = 0; x < pixmap.width() && !ink; ++x) {
                ink = pixmap.getColor(x, y) != SK_ColorWHITE;
            }
        }
        REQUIRE(ink);
    }
}

TEST_CASE("The executable directory holds the running executable", "[font][resources]") {
    #ifdef _WIN32
    CHECK(std::filesystem::exists(executableDirectory() / "psychic-ui-tests.exe"));
    #else
    CHECK(std::filesystem::exists(executableDirectory() / "psychic-ui-tests"));
    #endif
}

TEST_CASE("A relative font path resolves against the resource root", "[font][resources]") {
    auto manager = std::make_shared<StyleManager>();
    // The macro ends in res/: its parent's parent is the source directory
    manager->resourceRoot(std::filesystem::path(PSYCHIC_UI_TEST_RES).parent_path().parent_path());
    manager->loadFont("Inter", "res/fonts/Inter/Inter-Regular.ttf");

    sk_sp<SkTypeface> typeface = manager->font("Inter");
    REQUIRE(typeface);
    SkString name;
    typeface->getFamilyName(&name);
    CHECK(std::string{name.c_str()} == "Inter");
    CHECK(typeface->fontStyle().weight() == 400);
    // From the file, not a system Inter: under a name no system font has, only the file gives Inter
    manager->loadFont("Resource Root Inter", "res/fonts/Inter/Inter-Regular.ttf");
    SkString aliased;
    manager->font("Resource Root Inter")->getFamilyName(&aliased);
    CHECK(std::string{aliased.c_str()} == "Inter");

    CHECK(StyleManager{}.resourceRoot() == executableDirectory());
}
