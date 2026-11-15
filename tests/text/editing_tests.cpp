// Known bugs from docs/review-2026-11.md, tagged [!shouldfail]: remove the tag in the commit that fixes one.
#include <catch2/catch_test_macros.hpp>
#include <unicode/unistr.h>
#include <psychic-ui/components/Text.hpp>
#include <psychic-ui/components/TextInput.hpp>
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
            window->requestFocus(input.textDisplay().get(), true);
        }

        void type(UChar32 c) {
            window->handleType(icu::UnicodeString(c));
            sys.layout();
        }

        void key(Key k, Mod m = Mod{}) {
            window->handleKeyPress(k, m);
            sys.layout();
        }
    };
}

// C2: Text.cpp advances the caret by one UTF-16 unit after typing, and steps one unit for Backspace, Delete and the
// arrows
TEST_CASE("C2: typing a character outside the BMP puts the caret after it", "[text][editing][!shouldfail]") {
    Editing f;
    f.type(0x1F600);
    f.type('a');
    CHECK(f.input.text() == grinning + "a");
}

TEST_CASE("C2: Backspace removes a whole character outside the BMP", "[text][editing][!shouldfail]") {
    Editing f;
    f.input.text("x" + grinning);
    f.sys.layout();
    f.key(Key::END);
    f.key(Key::BACKSPACE);
    CHECK(f.input.text() == "x");
}

TEST_CASE("C2: Left steps over a whole character outside the BMP", "[text][editing][!shouldfail]") {
    Editing f;
    f.input.text(grinning);
    f.sys.layout();
    f.key(Key::END);
    f.key(Key::LEFT);
    f.type('a');
    CHECK(f.input.text() == "a" + grinning);
}

// C2: Text.cpp checks the caret before the selection, so a selection reaching the end (Delete) or the start
// (Backspace) isn't removed
TEST_CASE("C2: select all then Delete with the caret at the end empties the field", "[text][editing][!shouldfail]") {
    Editing f;
    for (char c: std::string("hello")) {
        f.type(static_cast<UChar32>(c));
    }
    f.key(Key::A, Mod{.ctrl = true});
    f.key(Key::DELETE);
    CHECK(f.input.text().empty());
}

TEST_CASE("C2: select all then Backspace with the caret at the start empties the field", "[text][editing][!shouldfail]") {
    Editing f;
    for (char c: std::string("hello")) {
        f.type(static_cast<UChar32>(c));
    }
    f.key(Key::HOME);
    f.key(Key::A, Mod{.ctrl = true});
    f.key(Key::BACKSPACE);
    CHECK(f.input.text().empty());
}
