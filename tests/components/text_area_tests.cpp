// TextArea on TextField: several lines, Enter inserts a new line, the caret scrolls into view in the skin's viewport
#include <catch2/catch_test_macros.hpp>
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include <unicode/umachine.h>
#include <unicode/unistr.h>
#include "include/core/SkRect.h"
#include <psychic-ui/components/Text.hpp>
#include <psychic-ui/components/TextArea.hpp>
#include <psychic-ui/psychic-ui.hpp>
#include <psychic-ui/skins/base/TextAreaSkinBase.hpp>
#include <psychic-ui/style/Values.hpp>
#include <psychic-ui/themes/Theme.hpp>
#include <psychic-ui/themes/pivotal.hpp>
#include "../backends/fake_system_window.hpp"

using namespace psychic_ui;
using namespace psychic_ui_tests;

namespace {
    struct Area {
        std::shared_ptr<TestWindow> window{std::make_shared<TestWindow>()};
        FakeSystemWindow            systemWindow{window, 800, 600};
        const Theme                 &theme{themes::pivotalDark};
        TextArea                    &area{window->appContainer().add<TextArea>("first line\nsecond line").size(200, 80)};
        std::vector<std::string>    changes{};

        Area() {
            window->appContainer().alignItems(Align::Start).padding(8);
            area.onChange([this](const std::string &text) { changes.push_back(text); });
            layout();
        }

        void layout() { systemWindow.layout(); }

        [[nodiscard]] Text &view() const { return *area.skin()->textView(); }

        /**
         * The view's viewport: its nearest scrolling ancestor
         */
        [[nodiscard]] Div &viewport() const {
            Div *div = view().parent();
            while (div && div->computedStyle().overflow() != Overflow::Scroll) {
                div = div->parent();
            }
            REQUIRE(div);
            return *div;
        }

        /**
         * Where the caret before the index is, in window coordinates, halfway down its line
         */
        [[nodiscard]] std::pair<float, float> at(unsigned int index) const {
            const SkRect caret = view().caretRect(index);
            int          x     = 0;
            int          y     = 0;
            view().localToGlobal(x, y, static_cast<int>(caret.fLeft), static_cast<int>(caret.centerY()));
            return {static_cast<float>(x), static_cast<float>(y)};
        }

        /**
         * Whether the caret's line shows whole in the viewport
         */
        [[nodiscard]] bool caretShows() const {
            const SkRect caret = view().caretRect(area.caret());
            int          x     = 0;
            int          top   = 0;
            int          bottom = 0;
            view().localToGlobal(x, top, 0, static_cast<int>(caret.fTop));
            view().localToGlobal(x, bottom, 0, static_cast<int>(caret.fBottom));
            int viewportTop = 0;
            viewport().localToGlobal(x, viewportTop);
            viewportTop -= viewport().scrollY(); // Its box, not its content
            return top >= viewportTop && bottom <= viewportTop + viewport().getHeight();
        }

        void press(std::pair<float, float> point, unsigned int clickCount = 0) {
            window->handleMouseButtonChange({.x = point.first, .y = point.second, .button = MouseButton::LEFT, .down = true, .clickCount = clickCount});
        }

        void move(std::pair<float, float> point) {
            window->handleMouseMove({.x = point.first, .y = point.second}, true);
        }

        void release(std::pair<float, float> point) {
            window->handleMouseButtonChange({.x = point.first, .y = point.second, .button = MouseButton::LEFT, .down = false});
            layout();
        }

        void key(Key key, Mod mod = Mod{}) {
            window->handleKeyPress(key, mod);
            layout();
        }

        void type(const std::string &text) {
            for (const char character: text) {
                window->handleType(icu::UnicodeString(static_cast<UChar32>(character)));
            }
            layout();
        }

        void focus() {
            window->requestFocus(&area, true);
            layout();
        }

        /**
         * Type lines until the viewport had to scroll
         */
        void typePastTheBottom() {
            focus();
            area.selection(22, 22); // The end
            for (int i = 0; i < 12; ++i) {
                key(Key::ENTER);
                type("line");
            }
        }
    };
}

TEST_CASE("TextArea: Enter inserts a new line", "[components][text-area]") {
    Area fixture;
    fixture.focus();
    CHECK(fixture.area.hasFocus());
    CHECK(fixture.area.computedStyle().borderColor() == fixture.theme.palette.accent); // `TextArea:focus`: the field has it
    CHECK(fixture.view().caretVisible());
    CHECK(fixture.view().multiline());

    fixture.key(Key::END); // Of the first line
    fixture.key(Key::ENTER);
    fixture.type("x");
    CHECK(fixture.area.text() == "first line\nx\nsecond line");
    CHECK(fixture.changes == std::vector<std::string>{"first line\n\nsecond line", "first line\nx\nsecond line"});
    CHECK(fixture.view().text() == fixture.area.text());
    CHECK(fixture.area.caret() == 12);
}

TEST_CASE("TextArea: the caret scrolls into view when typing past the bottom", "[components][text-area]") {
    Area fixture;
    REQUIRE(fixture.viewport().scrollY() == 0);
    fixture.typePastTheBottom();
    CHECK(fixture.viewport().scrollY() < 0);
    CHECK(fixture.caretShows());

    // Back to the start, the viewport scrolls back up
    fixture.key(Key::HOME, Mod{.ctrl = true});
    CHECK(fixture.area.caret() == 0);
    CHECK(fixture.viewport().scrollY() == 0);
    CHECK(fixture.caretShows());

    // And down again to the end
    fixture.key(Key::END, Mod{.ctrl = true});
    CHECK(fixture.viewport().scrollY() < 0);
    CHECK(fixture.caretShows());
}

TEST_CASE("TextArea: the mouse selects across lines", "[components][text-area][mouse]") {
    Area fixture;
    fixture.press(fixture.at(2));
    fixture.move(fixture.at(17));
    fixture.release(fixture.at(17));
    CHECK(fixture.area.hasFocus());
    CHECK(fixture.area.selection() == std::pair{2u, 17u});
    CHECK(fixture.view().selection() == std::pair{2u, 17u});

    // A press below the text, in the empty viewport, goes to the last line
    int x = 0;
    int y = 0;
    fixture.viewport().localToGlobal(x, y, fixture.viewport().getWidth() - 20, fixture.viewport().getHeight() - 4);
    fixture.press({static_cast<float>(x), static_cast<float>(y)}, 1);
    fixture.release({static_cast<float>(x), static_cast<float>(y)});
    CHECK(fixture.area.selection() == std::pair{22u, 22u});
}

TEST_CASE("TextArea: Up and Down keep the caret's column", "[components][text-area]") {
    Area fixture;
    fixture.area.text("abcdefgh\nab\nabcdefgh");
    fixture.focus();
    fixture.area.selection(6, 6);
    fixture.layout();

    fixture.key(Key::DOWN);
    CHECK(fixture.area.caret() == 11); // The short line's end
    fixture.key(Key::DOWN);
    CHECK(fixture.area.caret() == 18); // Back in the column
    fixture.key(Key::UP);
    CHECK(fixture.area.caret() == 11);
    fixture.key(Key::UP);
    CHECK(fixture.area.caret() == 6);
}

TEST_CASE("TextArea: a new skin keeps the text, the selection and the caret in view", "[components][text-area][skin]") {
    Area fixture;
    fixture.typePastTheBottom();
    const std::string text   = fixture.area.text();
    const int         scroll = fixture.viewport().scrollY();
    REQUIRE(scroll < 0);
    const Text *before = &fixture.view();

    fixture.area.skin(std::make_shared<TextAreaSkinBase>());
    fixture.layout();
    REQUIRE(&fixture.view() != before);
    CHECK(fixture.view().text() == text);
    CHECK(fixture.view().multiline());
    CHECK(fixture.view().caretVisible());
    CHECK(fixture.view().selection() == fixture.area.selection());
    CHECK(fixture.viewport().scrollY() == scroll);
    CHECK(fixture.caretShows());

    fixture.type("!");
    CHECK(fixture.area.text() == text + "!");
}

TEST_CASE("TextArea: the selection shows only while the field has the focus", "[components][text-area][skins]") {
    Area fixture;
    fixture.area.selection(0, 5);
    fixture.layout();
    CHECK(fixture.view().selection() == std::pair{0u, 5u});
    CHECK(fixture.view().computedStyle().selectionBackgroundColor().alpha() == 0); // Pivotal hides it

    fixture.focus();
    CHECK(fixture.view().computedStyle().selectionBackgroundColor() == fixture.theme.palette.accentSoft);

    fixture.window->requestFocus(nullptr);
    fixture.layout();
    CHECK(fixture.view().selection() == std::pair{0u, 5u}); // Kept, hidden
    CHECK(fixture.view().computedStyle().selectionBackgroundColor().alpha() == 0);
}
