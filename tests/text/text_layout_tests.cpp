#include <cmath>
#include <memory>
#include <string>
#include <yoga/Yoga.h>
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <SkPaint.h>
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
            .lineHeight(LINE_HEIGHT)
            .textAntiAlias(true);
        return node;
    }

    /**
     * Width of the widest line, measured with the same paint settings as TextBase
     */
    float textWidth(const std::shared_ptr<StyleManager> &styleManager, const std::string &text, float size = 14.0f, const std::string &font = FONT) {
        SkPaint paint{};
        paint.setAntiAlias(true);
        paint.setLCDRenderText(true);
        paint.setSubpixelText(true);
        paint.setTypeface(styleManager->font(font));
        paint.setTextSize(size);
        float       width = 0.0f;
        size_t      start = 0;
        while (start <= text.size()) {
            size_t end = text.find('\n', start);
            end = end == std::string::npos ? text.size() : end + 1;
            width = std::max(width, paint.measureText(text.c_str() + start, end - start));
            if (end == text.size()) {
                break;
            }
            start = end;
        }
        return std::ceil(width);
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
