#include <memory>
#include <cmath>
#include <string>
#include <catch2/catch_test_macros.hpp>
#include "include/core/SkCanvas.h"
#include "include/core/SkFont.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTypeface.h"
#include <psychic-ui/style/StyleManager.hpp>
#include <psychic-ui/themes/Theme.hpp>
#include <psychic-ui/utils/TextLayout.hpp>

using namespace psychic_ui;

namespace {
    // The theme's font paths are relative to a binary in the build directory: load from the sources instead
    std::string resource(std::string_view themePath) {
        constexpr std::string_view prefix = "../res/";
        REQUIRE(themePath.starts_with(prefix));
        return std::string{PSYCHIC_UI_TEST_RES} + std::string{themePath.substr(prefix.size())};
    }

    std::shared_ptr<StyleManager> withInter() {
        auto manager = std::make_shared<StyleManager>();
        const Typography type{};
        for (const Font &font: {type.sans, type.sansMedium, type.sansSemiBold}) {
            manager->loadFont(std::string{font.name}, resource(font.path));
        }
        return manager;
    }

    // Laid out as TextBase does with antialiased text: subpixel positioning, so slight hinting
    TextLayout layout(const std::shared_ptr<StyleManager> &manager, const std::string &family, float size) {
        TextLayout text{};
        text.setText(icu::UnicodeString::fromUTF8("The quick brown fox jumps over the lazy dog"));
        text.setStyle(
            TextLayoutStyle{manager->fontCollection(), family, size, size * 1.5f, 0.0f, SkFont::Edging::kSubpixelAntiAlias, true, TextAlign::Start},
            SK_ColorBLACK, SK_ColorBLACK
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
    REQUIRE(fontHinting(true) == SkFontHinting::kSlight);

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
        regular.paint(surface->getCanvas(), 0.0f, 0.0f);
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
