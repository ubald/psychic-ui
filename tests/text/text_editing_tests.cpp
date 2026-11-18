// The editing model alone: no Div, a fake geometry
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>
#include <catch2/catch_test_macros.hpp>
#include <unicode/unistr.h>
#include "include/core/SkRect.h"
#include <psychic-ui/psychic-ui.hpp>
#include <psychic-ui/text/TextEditing.hpp>
#include <psychic-ui/text/TextGeometry.hpp>

using namespace psychic_ui;
using Edit      = TextEditing::Edit;
using Direction = TextEditing::Direction;
using Unit      = TextEditing::Unit;

namespace {
    const std::string grinning = "\xF0\x9F\x98\x80"; // U+1F600, two UTF-16 units
    const std::string family   = "\xF0\x9F\x91\xA8\xE2\x80\x8D\xF0\x9F\x91\xA9\xE2\x80\x8D\xF0\x9F\x91\xA7"; // 👨‍👩‍👧, 8 units
    const std::string eAcute   = "e\xCC\x81"; // e and a combining acute accent, two units

    /**
     * A monospaced layout: every UTF-16 unit 10 wide, lines 20 high, broken at line returns only
     */
    class FakeGeometry final : public TextGeometry {
    public:
        explicit FakeGeometry(const TextEditing &editing) : _editing(&editing) {}

        SkRect caretRect(unsigned int index) const override {
            const auto &text   = _editing->text();
            int         line   = 0;
            int         column = 0;
            for (int32_t i = 0; std::cmp_less(i, index) && i < text.length(); ++i) {
                if (text.charAt(i) == u'\n') {
                    ++line;
                    column = 0;
                } else {
                    ++column;
                }
            }
            return SkRect::MakeXYWH(static_cast<float>(column) * 10.0f, static_cast<float>(line) * 20.0f, 0.0f, 20.0f);
        }

        unsigned int indexAt(float x, float y) const override {
            const auto &text   = _editing->text();
            const int   target = std::max(0, static_cast<int>(y / 20.0f));
            int         line   = 0;
            int32_t     i      = 0;
            while (line < target && i < text.length()) {
                if (text.charAt(i++) == u'\n') {
                    ++line;
                }
            }
            // Then along the line, to the nearest column, never past its line return
            const int column = std::max(0, static_cast<int>(std::lround(x / 10.0f)));
            for (int c = 0; c < column && i < text.length() && text.charAt(i) != u'\n'; ++c) {
                ++i;
            }
            return static_cast<unsigned int>(i);
        }

    private:
        const TextEditing *_editing;
    };

    class FakeClipboard final : public TextEditing::Clipboard {
    public:
        std::string text{};

        std::string clipboardText() const override { return text; }
        void setClipboardText(const std::string &t) override { text = t; }
    };

    struct Model {
        TextEditing   editing{};
        FakeGeometry  geometry{editing};
        FakeClipboard clipboard{};

        explicit Model(const std::string &text = "", bool multiline = false) {
            editing.setMultiline(multiline);
            editing.setText(icu::UnicodeString::fromUTF8(text));
        }

        Edit key(Key k, Mod m = Mod{}) {
            return editing.handleKey(k, m, geometry, clipboard);
        }

        Edit type(const std::string &str) {
            return editing.insert(icu::UnicodeString::fromUTF8(str));
        }

        std::string text() const {
            return editing.utf8();
        }
    };

    const Mod shift{.shift = true};
    const Mod ctrl{.ctrl = true};
    const Mod ctrlShift{.shift = true, .ctrl = true};
}

TEST_CASE("TextEditing inserts at the caret and moves it by what was inserted", "[text][editing-model]") {
    Model m{};
    CHECK(m.type("a") == (Edit::Text | Edit::Caret));
    m.type(grinning);
    m.type("b");
    CHECK(m.text() == "a" + grinning + "b");
    CHECK(m.editing.caret() == 4);
    // A whole string at once
    m.editing.select(1, 1);
    m.type("xyz");
    CHECK(m.text() == "axyz" + grinning + "b");
    CHECK(m.editing.caret() == 4);
}

TEST_CASE("TextEditing steps over whole graphemes", "[text][editing-model]") {
    SECTION("a character outside the BMP") {
        Model m{"x" + grinning + "y"};
        m.key(Key::END);
        CHECK(m.editing.caret() == 4);
        m.key(Key::LEFT);
        CHECK(m.editing.caret() == 3);
        m.key(Key::LEFT);
        CHECK(m.editing.caret() == 1);
        m.key(Key::RIGHT);
        CHECK(m.editing.caret() == 3);
        m.key(Key::BACKSPACE);
        CHECK(m.text() == "xy");
        CHECK(m.editing.caret() == 1);
    }

    SECTION("an emoji sequence joined by zero-width joiners") {
        Model m{family + "!"};
        m.key(Key::RIGHT);
        CHECK(m.editing.caret() == 8);
        m.key(Key::DELETE);
        CHECK(m.text() == family);
        m.key(Key::BACKSPACE);
        CHECK(m.text().empty());
    }

    SECTION("a letter and its combining mark") {
        Model m{eAcute + "t"};
        m.key(Key::RIGHT);
        CHECK(m.editing.caret() == 2);
        m.key(Key::LEFT);
        m.key(Key::DELETE);
        CHECK(m.text() == "t");
    }

    SECTION("the edges stay put") {
        Model m{"ab"};
        CHECK(m.key(Key::LEFT) == Edit::None);
        CHECK(m.key(Key::BACKSPACE) == Edit::None);
        m.key(Key::END);
        CHECK(m.key(Key::RIGHT) == Edit::None);
        CHECK(m.key(Key::DELETE) == Edit::None);
        CHECK(m.text() == "ab");
    }
}

TEST_CASE("TextEditing moves and erases by words", "[text][editing-model]") {
    Model m{"Hello brave  world"};

    SECTION("Ctrl+Right goes to the next word's end, past whitespace") {
        m.key(Key::RIGHT, ctrl);
        CHECK(m.editing.caret() == 5);
        m.key(Key::RIGHT, ctrl);
        CHECK(m.editing.caret() == 11);
        m.key(Key::RIGHT, ctrl);
        CHECK(m.editing.caret() == 18);
        CHECK(m.key(Key::RIGHT, ctrl) == Edit::None);
    }

    SECTION("Ctrl+Left goes to the previous word's start, past whitespace") {
        m.key(Key::END);
        m.key(Key::LEFT, ctrl);
        CHECK(m.editing.caret() == 13);
        m.key(Key::LEFT, ctrl);
        CHECK(m.editing.caret() == 6);
        // From inside a word, its own start
        m.editing.select(8, 8);
        m.key(Key::LEFT, ctrl);
        CHECK(m.editing.caret() == 6);
        m.key(Key::LEFT, ctrl);
        CHECK(m.editing.caret() == 0);
    }

    SECTION("Ctrl+Backspace and Ctrl+Delete erase as far") {
        m.editing.select(11, 11);
        m.key(Key::BACKSPACE, ctrl);
        CHECK(m.text() == "Hello   world");
        m.key(Key::DELETE, ctrl);
        CHECK(m.text() == "Hello ");
    }

    SECTION("a double click selects the word it lands on, not the space before it") {
        m.editing.selectAt(6, 2, false);
        CHECK(m.editing.selection() == std::make_pair(6u, 11u));
        m.editing.selectAt(8, 2, false);
        CHECK(m.editing.selection() == std::make_pair(6u, 11u));
        // Past the end, the last word
        m.editing.selectAt(18, 2, false);
        CHECK(m.editing.selection() == std::make_pair(13u, 18u));
    }
}

TEST_CASE("TextEditing selects with the mouse", "[text][editing-model]") {
    Model m{"Hello brave world. Second one."};

    SECTION("a press puts the caret, a drag selects from it") {
        CHECK(m.editing.selectAt(3, 1, false) == Edit::Caret);
        CHECK(m.editing.selection() == std::make_pair(3u, 3u));
        m.editing.dragTo(8);
        CHECK(m.editing.anchor() == 3);
        CHECK(m.editing.caret() == 8);
        m.editing.dragTo(1);
        CHECK(m.editing.selection() == std::make_pair(1u, 3u));
    }

    SECTION("Shift extends from the anchor") {
        m.editing.selectAt(3, 1, false);
        m.editing.selectAt(9, 1, true);
        CHECK(m.editing.anchor() == 3);
        CHECK(m.editing.caret() == 9);
    }

    SECTION("dragging after a double click keeps the word") {
        m.editing.selectAt(8, 2, false);
        m.editing.dragTo(14);
        CHECK(m.editing.selection() == std::make_pair(6u, 14u));
        m.editing.dragTo(2);
        CHECK(m.editing.selection() == std::make_pair(2u, 11u));
        CHECK(m.editing.caret() == 2);
    }

    SECTION("three clicks select the sentence, four everything") {
        m.editing.selectAt(3, 3, false);
        CHECK(m.editing.selection() == std::make_pair(0u, 19u));
        m.editing.selectAt(3, 4, false);
        CHECK(m.editing.selection() == std::make_pair(0u, 30u));
    }
}

TEST_CASE("TextEditing removes a selection first", "[text][editing-model]") {
    SECTION("select all then Delete with the caret at the end") {
        Model m{"hello"};
        m.key(Key::END);
        m.key(Key::A, ctrl);
        CHECK(m.editing.selection() == std::make_pair(0u, 5u));
        CHECK(m.key(Key::DELETE) == (Edit::Text | Edit::Caret));
        CHECK(m.text().empty());
    }

    SECTION("select all then Backspace with the caret at the start") {
        Model m{"hello"};
        m.key(Key::A, ctrl);
        m.key(Key::BACKSPACE);
        CHECK(m.text().empty());
    }

    SECTION("typing replaces it") {
        Model m{"hello"};
        m.editing.select(1, 4);
        m.type("ipp");
        CHECK(m.text() == "hippo");
        CHECK(m.editing.caret() == 4);
    }
}

TEST_CASE("TextEditing extends the selection with Shift", "[text][editing-model]") {
    Model m{"Hello world"};
    m.key(Key::RIGHT, shift);
    m.key(Key::RIGHT, shift);
    CHECK(m.editing.anchor() == 0);
    CHECK(m.editing.selection() == std::make_pair(0u, 2u));
    m.key(Key::RIGHT, ctrlShift);
    CHECK(m.editing.selection() == std::make_pair(0u, 5u));
    m.key(Key::END, shift);
    CHECK(m.editing.selection() == std::make_pair(0u, 11u));
    m.key(Key::LEFT, shift);
    CHECK(m.editing.selection() == std::make_pair(0u, 10u));
    // Back past the anchor, the other way
    m.key(Key::HOME);
    m.editing.select(5, 5);
    m.key(Key::LEFT, shift);
    m.key(Key::LEFT, shift);
    CHECK(m.editing.anchor() == 5);
    CHECK(m.editing.caret() == 3);
    CHECK(m.editing.selection() == std::make_pair(3u, 5u));
    // An arrow without Shift collapses it to its side
    m.key(Key::RIGHT);
    CHECK(m.editing.selection() == std::make_pair(5u, 5u));
    m.editing.select(2, 7);
    m.key(Key::LEFT);
    CHECK(m.editing.selection() == std::make_pair(2u, 2u));
}

TEST_CASE("TextEditing keeps one line on one line", "[text][editing-model]") {
    Model m{"Hello "};
    m.key(Key::END);
    m.clipboard.text = "big\nwide\r\nworld";
    m.key(Key::V, ctrl);
    CHECK(m.text() == "Hello big wide world");
    CHECK(m.editing.caret() == 20);
    CHECK(m.key(Key::ENTER) == Edit::None);

    SECTION("but not several") {
        Model lines{"", true};
        lines.clipboard.text = "a\nb";
        lines.key(Key::V, ctrl);
        lines.key(Key::ENTER);
        CHECK(lines.text() == "a\nb\n");
    }
}

TEST_CASE("TextEditing copies and cuts the selection", "[text][editing-model]") {
    Model m{"Hello wörld"};
    m.editing.select(6, 11);
    CHECK(m.key(Key::C, ctrl) == Edit::None);
    CHECK(m.clipboard.text == "wörld");
    m.key(Key::X, ctrl);
    CHECK(m.text() == "Hello ");
    // Without the modifier, nothing
    m.clipboard.text = "x";
    CHECK(m.key(Key::V) == Edit::None);
    CHECK(m.text() == "Hello ");
}

TEST_CASE("TextEditing moves between lines in the column it keeps", "[text][editing-model]") {
    // Columns: 6, 2, 8
    Model m{"abcdef\nab\nabcdefgh", true};
    m.editing.select(5, 5); // Column 5 on the first line

    m.key(Key::DOWN);
    CHECK(m.editing.caret() == 9); // The second line is shorter: its end
    m.key(Key::DOWN);
    CHECK(m.editing.caret() == 15); // Back in column 5
    m.key(Key::UP);
    m.key(Key::UP);
    CHECK(m.editing.caret() == 5);

    SECTION("past the first and last lines, the text's edges") {
        m.key(Key::UP);
        CHECK(m.editing.caret() == 0);
        m.key(Key::END, ctrl);
        m.key(Key::DOWN);
        CHECK(m.editing.caret() == 18);
    }

    SECTION("a horizontal move forgets the column") {
        m.key(Key::DOWN);
        m.key(Key::LEFT);
        m.key(Key::DOWN);
        CHECK(m.editing.caret() == 11);
    }

    SECTION("Home and End go to the line's edges, Ctrl to the text's") {
        m.key(Key::DOWN);
        m.key(Key::HOME);
        CHECK(m.editing.caret() == 7);
        m.key(Key::END);
        CHECK(m.editing.caret() == 9);
        m.key(Key::HOME, ctrl);
        CHECK(m.editing.caret() == 0);
    }

    SECTION("Shift+Down selects to the line below") {
        m.key(Key::DOWN, shift);
        CHECK(m.editing.selection() == std::make_pair(5u, 9u));
    }
}

TEST_CASE("Read-only TextEditing refuses edits but selects, moves and copies", "[text][editing-model]") {
    Model m{"Hello world"};
    m.editing.setReadOnly(true);
    CHECK(m.type("x") == Edit::None);
    CHECK(m.key(Key::DELETE) == Edit::None);
    CHECK(m.key(Key::RIGHT, shift) == Edit::Caret);
    CHECK(m.key(Key::BACKSPACE) == Edit::None);
    m.key(Key::A, ctrl);
    m.key(Key::C, ctrl);
    CHECK(m.clipboard.text == "Hello world");
    CHECK(m.key(Key::X, ctrl) == Edit::None);
    m.clipboard.text = "pasted";
    CHECK(m.key(Key::V, ctrl) == Edit::None);
    CHECK(m.text() == "Hello world");
    CHECK(m.editing.selection() == std::make_pair(0u, 11u));
}

TEST_CASE("TextEditing survives a move", "[text][editing-model]") {
    std::vector<TextEditing> models{};
    models.emplace_back().setText(icu::UnicodeString::fromUTF8("one two"));
    models.front().wordAt(1); // Its iterators made, on the first model's text
    models.emplace_back();      // Moves the first
    CHECK(models.front().wordAt(5) == std::make_pair(4u, 7u));
}
