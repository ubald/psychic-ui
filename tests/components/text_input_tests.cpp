// TextInput on TextField: the field takes the focus, the keys, typing and the mouse, and drives its skin's view
#include <catch2/catch_test_macros.hpp>
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include <unicode/umachine.h>
#include <unicode/unistr.h>
#include "include/core/SkRect.h"
#include <psychic-ui/components/Text.hpp>
#include <psychic-ui/components/TextInput.hpp>
#include <psychic-ui/psychic-ui.hpp>
#include <psychic-ui/style/Values.hpp>
#include <psychic-ui/themes/Theme.hpp>
#include <psychic-ui/themes/pivotal.hpp>
#include "../backends/fake_system_window.hpp"

using namespace psychic_ui;
using namespace psychic_ui_tests;

namespace {
    struct Field {
        std::shared_ptr<TestWindow> window{std::make_shared<TestWindow>()};
        FakeSystemWindow            systemWindow{window, 800, 600};
        const Theme                 &theme{themes::pivotalDark};
        TextInput                   &input{window->appContainer().add<TextInput>("Hello world").width(200)};
        std::vector<std::string>    changes{};

        Field() {
            window->appContainer().alignItems(Align::Start).padding(8);
            input.onChange([this](const std::string &text) { changes.push_back(text); });
            layout();
        }

        void layout() { systemWindow.layout(); }

        [[nodiscard]] Text &view() const { return *input.skin()->textView(); }

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

        void press(std::pair<float, float> point, unsigned int clickCount = 0, Mod mod = Mod{}) {
            window->handleMouseButtonChange({.x = point.first, .y = point.second, .button = MouseButton::LEFT, .down = true, .modifiers = mod, .clickCount = clickCount});
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
            window->requestFocus(&input, true);
            layout();
        }
    };

    Mod shift() {
        Mod mod{};
        mod.shift = true;
        return mod;
    }

    Mod ctrl() {
        Mod mod{};
        mod.ctrl = true;
        return mod;
    }
}

TEST_CASE("TextInput: a press on its text focuses the field, which matches :focus", "[components][text-input][focus]") {
    Field fixture;
    REQUIRE_FALSE(fixture.input.hasFocus());
    CHECK_FALSE(fixture.view().caretVisible());

    const auto end = fixture.at(11);
    fixture.press(end);
    fixture.release(end);
    CHECK(fixture.input.hasFocus());
    CHECK(fixture.input.computedStyle().borderColor() == fixture.theme.palette.accent); // `TextInput:focus`
    CHECK(fixture.view().caretVisible());
    CHECK(fixture.input.caret() == 11);

    // The keys reach the field, whatever div under it took the press
    fixture.type("!");
    CHECK(fixture.input.text() == "Hello world!");
}

TEST_CASE("TextInput: typing inserts at the caret and emits changed once per edit", "[components][text-input]") {
    Field fixture;
    fixture.focus();
    fixture.key(Key::END);
    CHECK(fixture.changes.empty()); // Moving the caret changes no text

    fixture.type("!?");
    CHECK(fixture.input.text() == "Hello world!?");
    CHECK(fixture.changes == std::vector<std::string>{"Hello world!", "Hello world!?"});
    CHECK(fixture.view().text() == "Hello world!?");
    CHECK(fixture.view().selection() == std::pair{13u, 13u});

    // Select all then Backspace: one edit
    fixture.key(Key::A, ctrl());
    fixture.key(Key::BACKSPACE);
    CHECK(fixture.input.text().empty());
    CHECK(fixture.changes.size() == 3);

    // Paste is one edit too
    fixture.window->setClipboardText("pasted");
    fixture.key(Key::V, ctrl());
    CHECK(fixture.input.text() == "pasted");
    CHECK(fixture.changes.size() == 4);

    // Setting the text isn't an edit
    fixture.input.text("set");
    fixture.layout();
    CHECK(fixture.changes.size() == 4);
    CHECK(fixture.view().text() == "set");
}

TEST_CASE("TextInput: Shift+arrows extend the selection from the anchor", "[components][text-input]") {
    Field fixture;
    fixture.focus();
    fixture.key(Key::HOME);
    fixture.key(Key::RIGHT, shift());
    fixture.key(Key::RIGHT, shift());
    CHECK(fixture.input.selection() == std::pair{0u, 2u});
    CHECK(fixture.view().selection() == std::pair{0u, 2u});

    // Word by word with Ctrl
    Mod both = shift();
    both.ctrl = true;
    fixture.key(Key::RIGHT, both);
    CHECK(fixture.input.selection() == std::pair{0u, 5u});

    // Back over the anchor
    fixture.key(Key::HOME, shift());
    CHECK(fixture.input.selection() == std::pair{0u, 0u});
    fixture.key(Key::END, shift());
    fixture.type("x");
    CHECK(fixture.input.text() == "x");
}

TEST_CASE("TextInput: the mouse selects by dragging, a word by a double click", "[components][text-input][mouse]") {
    Field fixture;
    fixture.press(fixture.at(0));
    fixture.move(fixture.at(5));
    fixture.release(fixture.at(5));
    CHECK(fixture.input.selection() == std::pair{0u, 5u});

    // The drag ended with the release
    fixture.move(fixture.at(11));
    CHECK(fixture.input.selection() == std::pair{0u, 5u});

    // Shift extends from the anchor
    fixture.press(fixture.at(8), 1, shift());
    fixture.release(fixture.at(8));
    CHECK(fixture.input.selection() == std::pair{0u, 8u});

    // Two clicks: the word under the press
    fixture.press(fixture.at(7), 2);
    fixture.release(fixture.at(7));
    CHECK(fixture.input.selection() == std::pair{6u, 11u});
}

TEST_CASE("TextInput: read-only, it selects and copies but refuses edits", "[components][text-input][read-only]") {
    Field fixture;
    fixture.input.readOnly(true);
    fixture.focus();
    CHECK(fixture.input.readOnly());
    CHECK_FALSE(fixture.view().caretVisible()); // No caret: nothing to type at

    fixture.type("x");
    fixture.key(Key::BACKSPACE);
    CHECK(fixture.input.text() == "Hello world");

    fixture.key(Key::A, ctrl());
    CHECK(fixture.input.selection() == std::pair{0u, 11u});
    fixture.key(Key::C, ctrl());
    CHECK(fixture.window->clipboardText() == "Hello world");
    fixture.key(Key::X, ctrl());
    CHECK(fixture.input.text() == "Hello world");
    fixture.key(Key::V, ctrl());
    CHECK(fixture.input.text() == "Hello world");
    CHECK(fixture.changes.empty());

    // Editable again
    fixture.input.readOnly(false);
    CHECK(fixture.view().caretVisible());
    fixture.type("x");
    CHECK(fixture.input.text() == "x");
}

TEST_CASE("TextInput: disabled, it ignores typing, keys and presses", "[components][text-input][disabled]") {
    Field fixture;
    fixture.input.enabled(false);
    fixture.layout();
    fixture.press(fixture.at(11));
    fixture.release(fixture.at(11));
    fixture.type("x");
    fixture.key(Key::BACKSPACE);
    fixture.key(Key::A, ctrl());
    CHECK(fixture.input.text() == "Hello world");
    CHECK(fixture.input.selection() == std::pair{0u, 0u});
    CHECK_FALSE(fixture.view().caretVisible());
    CHECK(fixture.changes.empty());

    fixture.input.enabled(true);
    fixture.focus();
    fixture.type("x");
    CHECK(fixture.input.text() == "xHello world");
}

TEST_CASE("TextInput: disabled, a press doesn't focus it and :focus doesn't match", "[components][text-input][disabled][focus]") {
    Field fixture;
    fixture.focus();
    REQUIRE(fixture.input.hasFocus());
    fixture.press(fixture.at(0)); // Its own press keeps the focus
    fixture.release(fixture.at(0));
    REQUIRE(fixture.input.hasFocus());

    fixture.input.enabled(false);
    fixture.layout();
    fixture.press(fixture.at(11));
    fixture.release(fixture.at(11));
    CHECK_FALSE(fixture.input.hasFocus()); // The press focused what is around it
    CHECK(fixture.window->appContainer().hasFocus());
    CHECK(fixture.input.computedStyle().borderColor() != fixture.theme.palette.accent); // Not `TextInput:focus`
    CHECK_FALSE(fixture.view().caretVisible());
}

TEST_CASE("TextInput: a new skin shows the text and the selection", "[components][text-input][skin]") {
    Field fixture;
    fixture.focus();
    fixture.input.selection(6, 11);
    const Text *before = &fixture.view();

    fixture.input.skin("bare");
    fixture.layout();
    REQUIRE(&fixture.view() != before);
    CHECK(fixture.input.skin()->classNames().contains("bare"));
    CHECK(fixture.view().text() == "Hello world");
    CHECK(fixture.view().selection() == std::pair{6u, 11u});
    CHECK(fixture.view().caretVisible());

    // Bare: no chrome, no padding
    CHECK(fixture.input.skin()->computedStyle().paddingLeft() == Length{0});
    CHECK(fixture.input.skin()->computedStyle().borderLeft() == 0.0f);
    CHECK(fixture.input.skin()->computedStyle().backgroundColor().alpha() == 0);

    fixture.type("there");
    CHECK(fixture.input.text() == "Hello there");
}

TEST_CASE("TextInput: the selection shows only while the field has the focus", "[components][text-input][skins]") {
    Field fixture;
    fixture.input.selection(0, 5);
    fixture.layout();
    CHECK(fixture.view().selection() == std::pair{0u, 5u});
    CHECK(fixture.view().computedStyle().selectionBackgroundColor().alpha() == 0); // Pivotal hides it

    fixture.focus();
    CHECK(fixture.view().computedStyle().selectionBackgroundColor() == fixture.theme.palette.accentSoft);
}

TEST_CASE("TextInput: one line scrolls to show its caret", "[components][text-input]") {
    Field fixture;
    fixture.input.text(std::string(80, 'w'));
    fixture.focus();
    CHECK(fixture.view().offsetX() == 0.0f);

    fixture.key(Key::END);
    CHECK(fixture.view().offsetX() > 0.0f);
    const SkRect caret = fixture.view().caretRect(80);
    CHECK(caret.fLeft <= static_cast<float>(fixture.view().getWidth()));
    CHECK(caret.fLeft >= 0.0f);

    fixture.key(Key::HOME);
    CHECK(fixture.view().offsetX() == 0.0f);

    // Deleting at the end scrolls back
    fixture.key(Key::END);
    fixture.key(Key::A, ctrl());
    fixture.key(Key::BACKSPACE);
    CHECK(fixture.view().offsetX() == 0.0f);
}
