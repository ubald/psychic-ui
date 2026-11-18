#include <cmath>
#include <memory>
#include <string>
#include <yoga/Yoga.h>
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "include/core/SkFont.h"
#include "include/core/SkSurface.h"
#include <psychic-ui/text/TextLayout.hpp>
#include <psychic-ui/style/StyleManager.hpp>
#include <psychic-ui/style/Style.hpp>
#include <psychic-ui/Div.hpp>
#include <psychic-ui/components/Text.hpp>
#include <psychic-ui/components/Label.hpp>

using namespace psychic_ui;
using Catch::Approx;

namespace {

    const std::string FONT      = "Ubuntu Regular";
    const std::string BOLD_FONT = "Ubuntu Bold";
    const float       LINE_HEIGHT = 20.0f;

    /**
     * Root of a headless tree: what Window::drawAll does, without a window or a canvas
     */
    class Root : public Div {
    public:
        explicit Root(std::shared_ptr<StyleManager> styleManager) {
            setStyleManager(std::move(styleManager));
        }

        void layout(float width, float height) {
            updateStyleRecursive();
            YGNodeCalculateLayout(_yogaNode, width, height, YGDirectionLTR);
            layoutUpdated();
        }

        using Div::invalidate;
    };

    std::shared_ptr<StyleManager> makeStyleManager() {
        auto styleManager = std::make_shared<StyleManager>();
        styleManager->loadFont(FONT, PSYCHIC_UI_TEST_FONTS "Ubuntu-Regular.ttf");
        styleManager->loadFont(BOLD_FONT, PSYCHIC_UI_TEST_FONTS "Ubuntu-Bold.ttf");
        return styleManager;
    }

    template<typename T>
    T &addText(Root &root, const std::string &text, float size = 14.0f, const std::string &font = FONT) {
        auto &node = root.add<T>(text);
        node.fontFamily(font)
            .fontSize(size)
            .lineHeight(LINE_HEIGHT);
        return node;
    }

    TextLayoutStyle layoutStyle(const std::shared_ptr<StyleManager> &styleManager, float size = 14.0f, const std::string &font = FONT) {
        return TextLayoutStyle{
            styleManager->fontCollection(), font, size, LINE_HEIGHT, 0.0f, FontSmoothing::Grayscale, TextAlign::Start
        };
    }

    /**
     * Width of the widest line, laid out with the same settings as Text
     */
    float textWidth(const std::shared_ptr<StyleManager> &styleManager, const std::string &text, float size = 14.0f, const std::string &font = FONT) {
        TextLayout layout{};
        layout.setText(icu::UnicodeString::fromUTF8(text));
        layout.setStyle(layoutStyle(styleManager, size, font));
        layout.setMultiline(true);
        return std::ceil(layout.maxContentWidth());
    }

}

TEST_CASE("Text and Label are measured to their content", "[text]") {
    auto styleManager = makeStyleManager();
    REQUIRE(styleManager->font(FONT));

    Root root{styleManager};
    root.width(400.0f)
        .height(300.0f);

    const std::string str = "Hello, measured world";
    const float       expected = textWidth(styleManager, str);
    REQUIRE(expected > 0);

    SECTION("in a row") {
        root.direction(Direction::Row);
        auto &text  = addText<Text>(root, str);
        auto &label = addText<Label>(root, str);
        root.layout(400, 300);

        REQUIRE(text.getWidth() == Approx(expected).margin(1));
        REQUIRE(label.getWidth() == Approx(expected).margin(1));
    }

    SECTION("in a column aligned to the start") {
        root.alignItems(Align::Start);
        auto &text  = addText<Text>(root, str);
        auto &label = addText<Label>(root, str);
        root.layout(400, 300);

        REQUIRE(text.getWidth() == Approx(expected).margin(1));
        REQUIRE(text.getHeight() == LINE_HEIGHT);
        REQUIRE(label.getWidth() == Approx(expected).margin(1));
        REQUIRE(label.getHeight() == LINE_HEIGHT);
    }

    SECTION("in a stretched column") {
        auto &text  = addText<Text>(root, str);
        auto &label = addText<Label>(root, str);
        root.layout(400, 300);

        REQUIRE(text.getWidth() == 400);
        REQUIRE(text.getHeight() == LINE_HEIGHT);
        REQUIRE(label.getWidth() == 400);
        REQUIRE(label.getHeight() == LINE_HEIGHT);
    }

    SECTION("empty text still gets a line") {
        root.alignItems(Align::Start);
        auto &text  = addText<Text>(root, "");
        auto &label = addText<Label>(root, "");
        root.layout(400, 300);

        REQUIRE(text.getWidth() == 0);
        REQUIRE(text.getHeight() == LINE_HEIGHT);
        REQUIRE(label.getWidth() == 0);
        REQUIRE(label.getHeight() == LINE_HEIGHT);
    }

    SECTION("multiline text is as wide as its widest line") {
        root.alignItems(Align::Start);
        const std::string lines = "short\na much longer line\nmid line";
        auto              &text  = addText<Text>(root, lines);
        text.multiline(true);
        root.layout(400, 300);

        REQUIRE(text.getWidth() == Approx(textWidth(styleManager, lines)).margin(1));
        REQUIRE(text.getHeight() == 3 * LINE_HEIGHT);
    }

    SECTION("multiline text wraps in a narrow column") {
        root.width(60.0f);
        auto &text = addText<Text>(root, str);
        text.multiline(true);
        root.layout(60, 300);

        REQUIRE(text.getWidth() == 60);
        REQUIRE(text.getHeight() >= 2 * LINE_HEIGHT);
        REQUIRE(std::fmod(text.getHeight(), LINE_HEIGHT) == 0);
    }
}

TEST_CASE("Text and Label are re-measured when their font changes through style", "[text]") {
    auto styleManager = makeStyleManager();

    Root root{styleManager};
    root.width(400.0f)
        .height(300.0f)
        .alignItems(Align::Start);

    const std::string str   = "Hello, measured world";
    auto              &text  = addText<Text>(root, str);
    auto              &label = addText<Label>(root, str);
    // Twice, so that anything settling over a second pass is done before the change
    root.layout(400, 300);
    root.layout(400, 300);
    REQUIRE(text.getWidth() == Approx(textWidth(styleManager, str)).margin(1));
    REQUIRE(label.getWidth() == Approx(textWidth(styleManager, str)).margin(1));

    SECTION("font size") {
        text.fontSize(16.0f);
        label.fontSize(16.0f);
        root.layout(400, 300);

        REQUIRE(text.getWidth() == Approx(textWidth(styleManager, str, 16.0f)).margin(1));
        REQUIRE(label.getWidth() == Approx(textWidth(styleManager, str, 16.0f)).margin(1));
    }

    SECTION("font family") {
        const float bold = textWidth(styleManager, str, 14.0f, BOLD_FONT);
        REQUIRE(bold != Approx(textWidth(styleManager, str)).margin(1));

        text.fontFamily(BOLD_FONT);
        label.fontFamily(BOLD_FONT);
        root.layout(400, 300);

        REQUIRE(text.getWidth() == Approx(bold).margin(1));
        REQUIRE(label.getWidth() == Approx(bold).margin(1));
    }

    SECTION("line height") {
        text.lineHeight(32.0f);
        label.lineHeight(32.0f);
        root.layout(400, 300);

        REQUIRE(text.getHeight() == 32);
        REQUIRE(label.getHeight() == 32);
    }

    SECTION("text") {
        text.text("Hi");
        label.text("Hi");
        root.layout(400, 300);

        REQUIRE(text.getWidth() == Approx(textWidth(styleManager, "Hi")).margin(1));
        REQUIRE(label.getWidth() == Approx(textWidth(styleManager, "Hi")).margin(1));
    }

    SECTION("invalidating a Div without a measure function is harmless") {
        root.invalidate();
        root.layout(400, 300);
        REQUIRE(text.getWidth() == Approx(textWidth(styleManager, str)).margin(1));
    }
}

TEST_CASE("Text layout lines, carets and hit testing", "[text]") {
    auto       styleManager = makeStyleManager();
    TextLayout layout{};
    layout.setStyle(layoutStyle(styleManager));
    layout.setMultiline(true);

    // The line a caret is on, from its rectangle
    auto lineOf = [&](unsigned int index) {
        return static_cast<unsigned int>(layout.caretRect(index).fTop / LINE_HEIGHT);
    };

    SECTION("hard line breaks") {
        layout.setText(icu::UnicodeString::fromUTF8("ab\ncd\n"));
        layout.layout(400.0f);
        REQUIRE(layout.lineCountAt(400.0f) == 3);
        REQUIRE(lineOf(2) == 0);
        REQUIRE(lineOf(3) == 1);
        REQUIRE(lineOf(5) == 1);
        REQUIRE(lineOf(6) == 2);
        REQUIRE(layout.caretRect(0).fLeft == 0.0f);
        REQUIRE(layout.caretRect(3).fLeft == 0.0f);
        REQUIRE(layout.caretRect(6).fLeft == 0.0f);
        // Past a line's end, before its line return
        REQUIRE(layout.indexAt(1000.0f, LINE_HEIGHT / 2) == 2);
        REQUIRE(layout.indexAt(-1000.0f, LINE_HEIGHT * 1.5f) == 3);
    }

    SECTION("carets advance along a line and hit testing finds them back") {
        layout.setText(icu::UnicodeString::fromUTF8("Hello world"));
        layout.layout(400.0f);
        float previous = -1.0f;
        for (unsigned int i = 0; i <= 11; ++i) {
            const SkRect caret = layout.caretRect(i);
            REQUIRE(caret.fTop == 0.0f);
            REQUIRE(caret.fLeft > previous);
            previous = caret.fLeft;
            REQUIRE(layout.indexAt(caret.fLeft + 0.5f, LINE_HEIGHT / 2) == i);
        }
        REQUIRE(previous == Approx(layout.maxContentWidth()).margin(0.5));
        // Past the end, below the last line
        REQUIRE(layout.indexAt(1000.0f, 1000.0f) == 11);
    }

    SECTION("wrapping keeps whole words and lines a line height apart") {
        const std::string str = "one two three four five six";
        layout.setText(icu::UnicodeString::fromUTF8(str));
        const float width = layout.maxContentWidth();
        REQUIRE(layout.lineCountAt(width + 1.0f) == 1);
        layout.layout(width / 2.0f);
        const auto lines = layout.lineCountAt(width / 2.0f);
        REQUIRE(lines >= 2);
        // Lines start on words
        for (unsigned int line = 1; line < lines; ++line) {
            const unsigned int start = layout.indexAt(0.0f, (line + 0.5f) * LINE_HEIGHT);
            REQUIRE(start > 0);
            REQUIRE(str[start - 1] == ' ');
            REQUIRE(lineOf(start) == line);
            REQUIRE(layout.caretRect(start).fLeft == 0.0f);
        }
    }

    SECTION("one line never wraps") {
        layout.setMultiline(false);
        layout.setText(icu::UnicodeString::fromUTF8("one two three four five six"));
        layout.layout(20.0f);
        REQUIRE(layout.lineCountAt(20.0f) == 1);
        REQUIRE(lineOf(27) == 0);
        const float fits = layout.fittingWidth(20.0f);
        REQUIRE(fits <= 20.0f);
        REQUIRE(fits > 0.0f);
    }

    SECTION("one line lays line returns out as spaces") {
        layout.setMultiline(false);
        layout.setText(icu::UnicodeString::fromUTF8("ab\ncd"));
        layout.layout(400.0f);
        REQUIRE(lineOf(4) == 0);
        REQUIRE(layout.caretRect(3).fLeft > layout.caretRect(2).fLeft);
    }

    SECTION("empty text has one line for the caret") {
        layout.setText(icu::UnicodeString{});
        layout.layout(400.0f);
        REQUIRE(layout.caretRect(0) == SkRect::MakeLTRB(0.0f, 0.0f, 0.0f, LINE_HEIGHT));
        REQUIRE(layout.indexAt(10.0f, 10.0f) == 0);
    }
}

TEST_CASE("Text layout caret rectangles, hit testing and selections", "[text]") {
    auto       styleManager = makeStyleManager();
    TextLayout layout{};
    layout.setStyle(layoutStyle(styleManager));
    layout.setMultiline(true);
    layout.setText(icu::UnicodeString::fromUTF8("ab\ncdef\ngh"));
    layout.layout(400.0f);

    SECTION("a caret is a line high, at the x of its index") {
        const SkRect first = layout.caretRect(0);
        CHECK(first.fLeft == 0.0f);
        CHECK(first.width() == 0.0f);
        CHECK(first.fTop == 0.0f);
        CHECK(first.height() == LINE_HEIGHT);

        const SkRect middle = layout.caretRect(5); // After "cd"
        CHECK(middle.fTop == LINE_HEIGHT);
        CHECK(middle.fLeft > layout.caretRect(3).fLeft);
        CHECK(layout.indexAt(middle.fLeft, middle.centerY()) == 5);
        CHECK(layout.caretRect(10).fTop == 2 * LINE_HEIGHT);
    }

    SECTION("a selection is painted over its text only, ending where its line's text ends") {
        auto  surface = SkSurfaces::Raster(SkImageInfo::MakeN32Premul(400, 100));
        auto *canvas  = surface->getCanvas();
        canvas->clear(SK_ColorWHITE);
        // From "b" to "g": all of the middle line
        layout.paintSelection(canvas, 10.0f, 10.0f, 1, 9, SK_ColorRED, SK_ColorBLACK);
        SkPixmap pixmap;
        REQUIRE(surface->peekPixels(&pixmap));
        const int   middleY = static_cast<int>(10.0f + LINE_HEIGHT) + 1; // Its top row, in the half-leading over the glyphs
        const float lineEnd = 10.0f + layout.caretRect(7).fLeft;
        CHECK(pixmap.getColor(11, middleY) == SK_ColorRED);
        CHECK(pixmap.getColor(static_cast<int>(lineEnd) - 1, middleY) == SK_ColorRED);
        // Not past the line's text (B7: middle lines went to the box's width plus its left inset)
        CHECK(pixmap.getColor(static_cast<int>(lineEnd) + 4, middleY) == SK_ColorWHITE);
        // Before "b" on the first line, nothing
        CHECK(pixmap.getColor(11, 11) == SK_ColorWHITE);
        // An empty selection paints nothing
        canvas->clear(SK_ColorWHITE);
        layout.paintSelection(canvas, 10.0f, 10.0f, 4, 4, SK_ColorRED, SK_ColorBLACK);
        CHECK(pixmap.getColor(11, middleY) == SK_ColorWHITE);
    }
}

TEST_CASE("Text is painted in the color given then, without building its paragraph again", "[text]") {
    auto       styleManager = makeStyleManager();
    TextLayout layout{};
    layout.setStyle(layoutStyle(styleManager));
    layout.setText(icu::UnicodeString::fromUTF8("Hello"));
    layout.layout(200.0f);

    auto surface = SkSurfaces::Raster(SkImageInfo::MakeN32Premul(200, 40));
    // The darkest pixel painted over white: a glyph's core, in the text's color
    auto inkOf = [&](SkColor color) {
        surface->getCanvas()->clear(SK_ColorWHITE);
        layout.paint(surface->getCanvas(), 0.0f, 0.0f, color);
        SkPixmap pixmap;
        REQUIRE(surface->peekPixels(&pixmap));
        SkColor darkest = SK_ColorWHITE;
        int     least   = 3 * 255;
        for (int y = 0; y < pixmap.height(); ++y) {
            for (int x = 0; x < pixmap.width(); ++x) {
                const SkColor c   = pixmap.getColor(x, y);
                const int     sum = static_cast<int>(SkColorGetR(c) + SkColorGetG(c) + SkColorGetB(c));
                if (sum < least) {
                    least   = sum;
                    darkest = c;
                }
            }
        }
        return darkest;
    };

    CHECK(inkOf(SK_ColorBLUE) == SK_ColorBLUE);
    const auto built = TextLayout::paragraphsBuilt();
    CHECK(inkOf(SK_ColorRED) == SK_ColorRED);
    CHECK(inkOf(SK_ColorBLUE) == SK_ColorBLUE);
    CHECK(TextLayout::paragraphsBuilt() == built);
}

TEST_CASE("One line too long ends in an ellipsis that fits, or overflows", "[text]") {
    auto       styleManager = makeStyleManager();
    TextLayout layout{};
    auto       style = layoutStyle(styleManager);
    layout.setText(icu::UnicodeString::fromUTF8("one two three four five six"));
    layout.setStyle(style);
    const float whole = layout.maxContentWidth();
    layout.layout(60.0f);

    auto surface = SkSurfaces::Raster(SkImageInfo::MakeN32Premul(400, 40));
    // The rightmost column with ink
    auto inkEnd = [&] {
        surface->getCanvas()->clear(SK_ColorWHITE);
        layout.paint(surface->getCanvas(), 0.0f, 0.0f, SK_ColorBLACK);
        SkPixmap pixmap;
        REQUIRE(surface->peekPixels(&pixmap));
        int end = -1;
        for (int y = 0; y < pixmap.height(); ++y) {
            for (int x = 0; x < pixmap.width(); ++x) {
                if (pixmap.getColor(x, y) != SK_ColorWHITE) {
                    end = std::max(end, x);
                }
            }
        }
        return end;
    };

    SECTION("clipped, it runs past the width") {
        CHECK(inkEnd() > 60);
    }

    SECTION("with an ellipsis, it stops within the width and still measures as a whole") {
        style.overflow = TextOverflow::Ellipsis;
        CHECK(layout.setStyle(style));
        const int end = inkEnd();
        CHECK(end > 30);
        CHECK(end < 60);
        CHECK(layout.lineCountAt(60.0f) == 1);
        CHECK(layout.maxContentWidth() == Approx(whole));
    }

    SECTION("several lines wrap instead") {
        style.overflow = TextOverflow::Ellipsis;
        layout.setStyle(style);
        layout.setMultiline(true);
        CHECK(layout.lineCountAt(60.0f) > 1);
    }
}

TEST_CASE("A Label keeps its text as set and is as tall as its line height", "[text]") {
    auto styleManager = makeStyleManager();
    Root root{styleManager};
    root.alignItems(Align::Start);

    auto &label = root.add<Label>("two\nlines");
    label.fontFamily(FONT)
        .fontSize(14.0f)
        .lineHeight(8.0f); // Less than the font's spacing: the glyphs overflow it, as in CSS
    root.layout(400, 300);

    CHECK(label.text() == "two\nlines");
    CHECK(label.getHeight() == 8.0f);
    CHECK_FALSE(label.selectable());
    CHECK_FALSE(label.multiline());
    CHECK(label.tags().back() == "label");
}
