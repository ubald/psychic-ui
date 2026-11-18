#include <cmath>
#include <memory>
#include <string>
#include <yoga/Yoga.h>
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "include/core/SkFont.h"
#include <psychic-ui/utils/TextLayout.hpp>
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
        node.fontFamily(FONT)
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
     * Width of the widest line, laid out with the same settings as TextBase
     */
    float textWidth(const std::shared_ptr<StyleManager> &styleManager, const std::string &text, float size = 14.0f, const std::string &font = FONT) {
        TextLayout layout{};
        layout.setText(icu::UnicodeString::fromUTF8(text));
        layout.setStyle(layoutStyle(styleManager, size, font), SK_ColorBLACK, SK_ColorBLACK);
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
    layout.setStyle(layoutStyle(styleManager), SK_ColorBLACK, SK_ColorBLACK);
    layout.setMultiline(true);

    SECTION("hard line breaks") {
        layout.setText(icu::UnicodeString::fromUTF8("ab\ncd\n"));
        layout.layout(400.0f);
        REQUIRE(layout.lineCount() == 3);
        REQUIRE(layout.lineStart(1) == 3);
        REQUIRE(layout.lineEnd(0) == 2);
        REQUIRE(layout.lineEnd(1) == 5);
        REQUIRE(layout.lineFromIndex(2) == 0);
        REQUIRE(layout.lineFromIndex(3) == 1);
        REQUIRE(layout.lineFromIndex(6) == 2);
        REQUIRE(layout.posFromIndex(0).second == 0.0f);
        REQUIRE(layout.posFromIndex(6) == std::make_pair(2u, 0.0f));
    }

    SECTION("carets advance along a line and hit testing finds them back") {
        layout.setText(icu::UnicodeString::fromUTF8("Hello world"));
        layout.layout(400.0f);
        REQUIRE(layout.lineCount() == 1);
        float previous = -1.0f;
        for (unsigned int i = 0; i <= 11; ++i) {
            auto [line, x] = layout.posFromIndex(i);
            REQUIRE(line == 0);
            REQUIRE(x > previous);
            previous = x;
            REQUIRE(layout.indexFromPos(x + 0.5f, LINE_HEIGHT / 2) == i);
        }
        REQUIRE(previous == Approx(layout.maxContentWidth()).margin(0.5));
        // Past the end, below the last line
        REQUIRE(layout.indexFromPos(1000.0f, 1000.0f) == 11);
    }

    SECTION("wrapping keeps whole words and lines a line height apart") {
        layout.setText(icu::UnicodeString::fromUTF8("one two three four five six"));
        float width = layout.maxContentWidth();
        REQUIRE(layout.lineCountAt(width + 1.0f) == 1);
        layout.layout(width / 2.0f);
        REQUIRE(layout.lineCount() >= 2);
        // Lines start on words
        for (unsigned int line = 1; line < layout.lineCount(); ++line) {
            auto start = layout.lineStart(line);
            REQUIRE(start > 0);
            REQUIRE(layout.previousWordBoundary(start + 1) == start);
            REQUIRE(layout.lineFromIndex(start) == line);
            REQUIRE(layout.indexFromPos(0.0f, (line + 0.5f) * LINE_HEIGHT) == start);
        }
    }

    SECTION("one line never wraps") {
        layout.setMultiline(false);
        layout.setText(icu::UnicodeString::fromUTF8("one two three four five six"));
        layout.layout(20.0f);
        REQUIRE(layout.lineCount() == 1);
        REQUIRE(layout.lineCountAt(20.0f) == 1);
        float fits = layout.fittingWidth(20.0f);
        REQUIRE(fits <= 20.0f);
        REQUIRE(fits > 0.0f);
    }

    SECTION("one line lays line returns out as spaces") {
        layout.setMultiline(false);
        layout.setText(icu::UnicodeString::fromUTF8("ab\ncd"));
        layout.layout(400.0f);
        REQUIRE(layout.lineCount() == 1);
        REQUIRE(layout.lineFromIndex(4) == 0);
        REQUIRE(layout.posFromIndex(3).second > layout.posFromIndex(2).second);
    }

    SECTION("empty text has one line for the caret") {
        layout.setText(icu::UnicodeString{});
        layout.layout(400.0f);
        REQUIRE(layout.lineCount() == 1);
        REQUIRE(layout.posFromIndex(0) == std::make_pair(0u, 0.0f));
        REQUIRE(layout.indexFromPos(10.0f, 10.0f) == 0);
    }

    SECTION("words and sentences") {
        layout.setText(icu::UnicodeString::fromUTF8("Hello brave world. Second one."));
        REQUIRE(layout.wordAtIndex(8) == std::make_pair(6u, 11u));
        REQUIRE(layout.nextWordBoundary(6) == 11);
        REQUIRE(layout.sentenceAtIndex(3) == std::make_pair(0u, 19u));
    }
}
