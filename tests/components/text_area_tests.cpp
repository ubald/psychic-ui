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

        Text &view() const { return *area.skin()->textView(); }

        /**
         * The view's viewport: its nearest scrolling ancestor
         */
        Div &viewport() const {
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
        std::pair<float, float> at(unsigned int index) const {
            const SkRect caret = view().caretRect(index);
            int          x     = 0;
            int          y     = 0;
            view().localToGlobal(x, y, static_cast<int>(caret.fLeft), static_cast<int>(caret.centerY()));
            return {static_cast<float>(x), static_cast<float>(y)};
        }

        /**
         * Whether the caret's line shows whole in the viewport
         */
        bool caretShows() const {
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

        void press(std::pair<float, float> p, unsigned int clickCount = 0) {
            window->handleMouseButtonChange({.x = p.first, .y = p.second, .button = MouseButton::LEFT, .down = true, .clickCount = clickCount});
        }

        void move(std::pair<float, float> p) {
            window->handleMouseMove({.x = p.first, .y = p.second}, true);
        }

        void release(std::pair<float, float> p) {
            window->handleMouseButtonChange({.x = p.first, .y = p.second, .button = MouseButton::LEFT, .down = false});
            layout();
        }

        void key(Key key, Mod mod = Mod{}) {
            window->handleKeyPress(key, mod);
            layout();
        }

        void type(const std::string &text) {
            for (const char c: text) {
                window->handleType(icu::UnicodeString(static_cast<UChar32>(c)));
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
    Area f;
    f.focus();
    CHECK(f.area.hasFocus());
    CHECK(f.area.computedStyle().borderColor() == f.theme.palette.accent); // `TextArea:focus`: the field has it
    CHECK(f.view().caretVisible());
    CHECK(f.view().multiline());

    f.key(Key::END); // Of the first line
    f.key(Key::ENTER);
    f.type("x");
    CHECK(f.area.text() == "first line\nx\nsecond line");
    CHECK(f.changes == std::vector<std::string>{"first line\n\nsecond line", "first line\nx\nsecond line"});
    CHECK(f.view().text() == f.area.text());
    CHECK(f.area.caret() == 12);
}

TEST_CASE("TextArea: the caret scrolls into view when typing past the bottom", "[components][text-area]") {
    Area f;
    REQUIRE(f.viewport().scrollY() == 0);
    f.typePastTheBottom();
    CHECK(f.viewport().scrollY() < 0);
    CHECK(f.caretShows());

    // Back to the start, the viewport scrolls back up
    f.key(Key::HOME, Mod{.ctrl = true});
    CHECK(f.area.caret() == 0);
    CHECK(f.viewport().scrollY() == 0);
    CHECK(f.caretShows());

    // And down again to the end
    f.key(Key::END, Mod{.ctrl = true});
    CHECK(f.viewport().scrollY() < 0);
    CHECK(f.caretShows());
}

TEST_CASE("TextArea: the mouse selects across lines", "[components][text-area][mouse]") {
    Area f;
    f.press(f.at(2));
    f.move(f.at(17));
    f.release(f.at(17));
    CHECK(f.area.hasFocus());
    CHECK(f.area.selection() == std::pair{2u, 17u});
    CHECK(f.view().selection() == std::pair{2u, 17u});

    // A press below the text, in the empty viewport, goes to the last line
    int x = 0;
    int y = 0;
    f.viewport().localToGlobal(x, y, f.viewport().getWidth() - 20, f.viewport().getHeight() - 4);
    f.press({static_cast<float>(x), static_cast<float>(y)}, 1);
    f.release({static_cast<float>(x), static_cast<float>(y)});
    CHECK(f.area.selection() == std::pair{22u, 22u});
}

TEST_CASE("TextArea: Up and Down keep the caret's column", "[components][text-area]") {
    Area f;
    f.area.text("abcdefgh\nab\nabcdefgh");
    f.focus();
    f.area.selection(6, 6);
    f.layout();

    f.key(Key::DOWN);
    CHECK(f.area.caret() == 11); // The short line's end
    f.key(Key::DOWN);
    CHECK(f.area.caret() == 18); // Back in the column
    f.key(Key::UP);
    CHECK(f.area.caret() == 11);
    f.key(Key::UP);
    CHECK(f.area.caret() == 6);
}

TEST_CASE("TextArea: a new skin keeps the text, the selection and the caret in view", "[components][text-area][skin]") {
    Area f;
    f.typePastTheBottom();
    const std::string text   = f.area.text();
    const int         scroll = f.viewport().scrollY();
    REQUIRE(scroll < 0);
    const Text *before = &f.view();

    f.area.skin(std::make_shared<TextAreaSkinBase>());
    f.layout();
    REQUIRE(&f.view() != before);
    CHECK(f.view().text() == text);
    CHECK(f.view().multiline());
    CHECK(f.view().caretVisible());
    CHECK(f.view().selection() == f.area.selection());
    CHECK(f.viewport().scrollY() == scroll);
    CHECK(f.caretShows());

    f.type("!");
    CHECK(f.area.text() == text + "!");
}

TEST_CASE("TextArea: the selection shows only while the field has the focus", "[components][text-area][skins]") {
    Area f;
    f.area.selection(0, 5);
    f.layout();
    CHECK(f.view().selection() == std::pair{0u, 5u});
    CHECK(f.view().computedStyle().selectionBackgroundColor().alpha() == 0); // Pivotal hides it

    f.focus();
    CHECK(f.view().computedStyle().selectionBackgroundColor() == f.theme.palette.accentSoft);

    f.window->requestFocus(nullptr);
    f.layout();
    CHECK(f.view().selection() == std::pair{0u, 5u}); // Kept, hidden
    CHECK(f.view().computedStyle().selectionBackgroundColor().alpha() == 0);
}
