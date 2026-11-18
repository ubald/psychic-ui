// C2 from review-2026-11.md, through a TextInput: fixed by TextEditing
#include <memory>
#include <string>
#include <catch2/catch_test_macros.hpp>
#include <unicode/umachine.h>
#include <unicode/unistr.h>
#include <psychic-ui/components/TextInput.hpp>
#include <psychic-ui/psychic-ui.hpp>
#include "../backends/fake_system_window.hpp"

using namespace psychic_ui;
using namespace psychic_ui_tests;

namespace {
    const std::string grinning = "\xF0\x9F\x98\x80"; // U+1F600, two UTF-16 units

    struct Editing {
        std::shared_ptr<TestWindow> window{std::make_shared<TestWindow>()};
        FakeSystemWindow            sys{window, 800, 600};
        TextInput                  &input{window->appContainer().add<TextInput>("")};

        Editing() {
            sys.layout();
            window->requestFocus(&input, true);
        }

        void type(UChar32 character) {
            window->handleType(icu::UnicodeString(character));
            sys.layout();
        }

        void key(Key k, Mod mod = Mod{}) {
            window->handleKeyPress(k, mod);
            sys.layout();
        }
    };
}

// C2: Text.cpp advances the caret by one UTF-16 unit after typing, and steps one unit for Backspace, Delete and the
// arrows
TEST_CASE("C2: typing a character outside the BMP puts the caret after it", "[text][editing]") {
    Editing fixture;
    fixture.type(0x1F600);
    fixture.type('a');
    CHECK(fixture.input.text() == grinning + "a");
}

TEST_CASE("C2: Backspace removes a whole character outside the BMP", "[text][editing]") {
    Editing fixture;
    fixture.input.text("x" + grinning);
    fixture.sys.layout();
    fixture.key(Key::END);
    fixture.key(Key::BACKSPACE);
    CHECK(fixture.input.text() == "x");
}

TEST_CASE("C2: Left steps over a whole character outside the BMP", "[text][editing]") {
    Editing fixture;
    fixture.input.text(grinning);
    fixture.sys.layout();
    fixture.key(Key::END);
    fixture.key(Key::LEFT);
    fixture.type('a');
    CHECK(fixture.input.text() == "a" + grinning);
}

// C2: Text.cpp checks the caret before the selection, so a selection reaching the end (Delete) or the start
// (Backspace) isn't removed
TEST_CASE("C2: select all then Delete with the caret at the end empties the field", "[text][editing]") {
    Editing fixture;
    for (const char character: std::string("hello")) {
        fixture.type(static_cast<UChar32>(character));
    }
    fixture.key(Key::A, Mod{.ctrl = true});
    fixture.key(Key::DELETE);
    CHECK(fixture.input.text().empty());
}

TEST_CASE("C2: select all then Backspace with the caret at the start empties the field", "[text][editing]") {
    Editing fixture;
    for (const char character: std::string("hello")) {
        fixture.type(static_cast<UChar32>(character));
    }
    fixture.key(Key::HOME);
    fixture.key(Key::A, Mod{.ctrl = true});
    fixture.key(Key::BACKSPACE);
    CHECK(fixture.input.text().empty());
}
