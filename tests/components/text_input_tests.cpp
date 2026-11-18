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

        void press(std::pair<float, float> p, unsigned int clickCount = 0, Mod mod = Mod{}) {
            window->handleMouseButtonChange({.x = p.first, .y = p.second, .button = MouseButton::LEFT, .down = true, .modifiers = mod, .clickCount = clickCount});
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
    Field f;
    REQUIRE_FALSE(f.input.hasFocus());
    CHECK_FALSE(f.view().caretVisible());

    const auto end = f.at(11);
    f.press(end);
    f.release(end);
    CHECK(f.input.hasFocus());
    CHECK(f.input.computedStyle().borderColor() == f.theme.palette.accent); // `TextInput:focus`
    CHECK(f.view().caretVisible());
    CHECK(f.input.caret() == 11);

    // The keys reach the field, whatever div under it took the press
    f.type("!");
    CHECK(f.input.text() == "Hello world!");
}

TEST_CASE("TextInput: typing inserts at the caret and emits changed once per edit", "[components][text-input]") {
    Field f;
    f.focus();
    f.key(Key::END);
    CHECK(f.changes.empty()); // Moving the caret changes no text

    f.type("!?");
    CHECK(f.input.text() == "Hello world!?");
    CHECK(f.changes == std::vector<std::string>{"Hello world!", "Hello world!?"});
    CHECK(f.view().text() == "Hello world!?");
    CHECK(f.view().selection() == std::pair{13u, 13u});

    // Select all then Backspace: one edit
    f.key(Key::A, ctrl());
    f.key(Key::BACKSPACE);
    CHECK(f.input.text().empty());
    CHECK(f.changes.size() == 3);

    // Paste is one edit too
    f.window->setClipboardText("pasted");
    f.key(Key::V, ctrl());
    CHECK(f.input.text() == "pasted");
    CHECK(f.changes.size() == 4);

    // Setting the text isn't an edit
    f.input.text("set");
    f.layout();
    CHECK(f.changes.size() == 4);
    CHECK(f.view().text() == "set");
}

TEST_CASE("TextInput: Shift+arrows extend the selection from the anchor", "[components][text-input]") {
    Field f;
    f.focus();
    f.key(Key::HOME);
    f.key(Key::RIGHT, shift());
    f.key(Key::RIGHT, shift());
    CHECK(f.input.selection() == std::pair{0u, 2u});
    CHECK(f.view().selection() == std::pair{0u, 2u});

    // Word by word with Ctrl
    Mod both = shift();
    both.ctrl = true;
    f.key(Key::RIGHT, both);
    CHECK(f.input.selection() == std::pair{0u, 5u});

    // Back over the anchor
    f.key(Key::HOME, shift());
    CHECK(f.input.selection() == std::pair{0u, 0u});
    f.key(Key::END, shift());
    f.type("x");
    CHECK(f.input.text() == "x");
}

TEST_CASE("TextInput: the mouse selects by dragging, a word by a double click", "[components][text-input][mouse]") {
    Field f;
    f.press(f.at(0));
    f.move(f.at(5));
    f.release(f.at(5));
    CHECK(f.input.selection() == std::pair{0u, 5u});

    // The drag ended with the release
    f.move(f.at(11));
    CHECK(f.input.selection() == std::pair{0u, 5u});

    // Shift extends from the anchor
    f.press(f.at(8), 1, shift());
    f.release(f.at(8));
    CHECK(f.input.selection() == std::pair{0u, 8u});

    // Two clicks: the word under the press
    f.press(f.at(7), 2);
    f.release(f.at(7));
    CHECK(f.input.selection() == std::pair{6u, 11u});
}

TEST_CASE("TextInput: read-only, it selects and copies but refuses edits", "[components][text-input][read-only]") {
    Field f;
    f.input.readOnly(true);
    f.focus();
    CHECK(f.input.readOnly());
    CHECK_FALSE(f.view().caretVisible()); // No caret: nothing to type at

    f.type("x");
    f.key(Key::BACKSPACE);
    CHECK(f.input.text() == "Hello world");

    f.key(Key::A, ctrl());
    CHECK(f.input.selection() == std::pair{0u, 11u});
    f.key(Key::C, ctrl());
    CHECK(f.window->clipboardText() == "Hello world");
    f.key(Key::X, ctrl());
    CHECK(f.input.text() == "Hello world");
    f.key(Key::V, ctrl());
    CHECK(f.input.text() == "Hello world");
    CHECK(f.changes.empty());

    // Editable again
    f.input.readOnly(false);
    CHECK(f.view().caretVisible());
    f.type("x");
    CHECK(f.input.text() == "x");
}

TEST_CASE("TextInput: disabled, it ignores typing, keys and presses", "[components][text-input][disabled]") {
    Field f;
    f.input.enabled(false);
    f.layout();
    f.press(f.at(11));
    f.release(f.at(11));
    f.type("x");
    f.key(Key::BACKSPACE);
    f.key(Key::A, ctrl());
    CHECK(f.input.text() == "Hello world");
    CHECK(f.input.selection() == std::pair{0u, 0u});
    CHECK_FALSE(f.view().caretVisible());
    CHECK(f.changes.empty());

    f.input.enabled(true);
    f.focus();
    f.type("x");
    CHECK(f.input.text() == "xHello world");
}

TEST_CASE("TextInput: disabled, a press doesn't focus it and :focus doesn't match", "[components][text-input][disabled][focus]") {
    Field f;
    f.focus();
    REQUIRE(f.input.hasFocus());
    f.press(f.at(0)); // Its own press keeps the focus
    f.release(f.at(0));
    REQUIRE(f.input.hasFocus());

    f.input.enabled(false);
    f.layout();
    f.press(f.at(11));
    f.release(f.at(11));
    CHECK_FALSE(f.input.hasFocus()); // The press focused what is around it
    CHECK(f.window->appContainer().hasFocus());
    CHECK(f.input.computedStyle().borderColor() != f.theme.palette.accent); // Not `TextInput:focus`
    CHECK_FALSE(f.view().caretVisible());
}

TEST_CASE("TextInput: a new skin shows the text and the selection", "[components][text-input][skin]") {
    Field f;
    f.focus();
    f.input.selection(6, 11);
    const Text *before = &f.view();

    f.input.skin("bare");
    f.layout();
    REQUIRE(&f.view() != before);
    CHECK(f.input.skin()->classNames().contains("bare"));
    CHECK(f.view().text() == "Hello world");
    CHECK(f.view().selection() == std::pair{6u, 11u});
    CHECK(f.view().caretVisible());

    // Bare: no chrome, no padding
    CHECK(f.input.skin()->computedStyle().paddingLeft() == Length{0});
    CHECK(f.input.skin()->computedStyle().borderLeft() == 0.0f);
    CHECK(f.input.skin()->computedStyle().backgroundColor().alpha() == 0);

    f.type("there");
    CHECK(f.input.text() == "Hello there");
}

TEST_CASE("TextInput: the selection shows only while the field has the focus", "[components][text-input][skins]") {
    Field f;
    f.input.selection(0, 5);
    f.layout();
    CHECK(f.view().selection() == std::pair{0u, 5u});
    CHECK(f.view().computedStyle().selectionBackgroundColor().alpha() == 0); // Pivotal hides it

    f.focus();
    CHECK(f.view().computedStyle().selectionBackgroundColor() == f.theme.palette.accentSoft);
}

TEST_CASE("TextInput: one line scrolls to show its caret", "[components][text-input]") {
    Field f;
    f.input.text(std::string(80, 'w'));
    f.focus();
    CHECK(f.view().offsetX() == 0.0f);

    f.key(Key::END);
    CHECK(f.view().offsetX() > 0.0f);
    const SkRect caret = f.view().caretRect(80);
    CHECK(caret.fLeft <= static_cast<float>(f.view().getWidth()));
    CHECK(caret.fLeft >= 0.0f);

    f.key(Key::HOME);
    CHECK(f.view().offsetX() == 0.0f);

    // Deleting at the end scrolls back
    f.key(Key::END);
    f.key(Key::A, ctrl());
    f.key(Key::BACKSPACE);
    CHECK(f.view().offsetX() == 0.0f);
}
