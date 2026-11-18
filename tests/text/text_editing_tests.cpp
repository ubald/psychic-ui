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

        [[nodiscard]] SkRect caretRect(unsigned int index) const override {
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

        [[nodiscard]] unsigned int indexAt(float x, float y) const override {
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
            for (int step = 0; step < column && i < text.length() && text.charAt(i) != u'\n'; ++step) {
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

        [[nodiscard]] std::string clipboardText() const override { return text; }
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

        Edit key(Key k, Mod mod = Mod{}) {
            return editing.handleKey(k, mod, geometry, clipboard);
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
    Model model{};
    CHECK(model.type("a") == (Edit::Text | Edit::Caret));
    model.type(grinning);
    model.type("b");
    CHECK(model.text() == "a" + grinning + "b");
    CHECK(model.editing.caret() == 4);
    // A whole string at once
    model.editing.select(1, 1);
    model.type("xyz");
    CHECK(model.text() == "axyz" + grinning + "b");
    CHECK(model.editing.caret() == 4);
}

TEST_CASE("TextEditing steps over whole graphemes", "[text][editing-model]") {
    SECTION("a character outside the BMP") {
        Model model{"x" + grinning + "y"};
        model.key(Key::END);
        CHECK(model.editing.caret() == 4);
        model.key(Key::LEFT);
        CHECK(model.editing.caret() == 3);
        model.key(Key::LEFT);
        CHECK(model.editing.caret() == 1);
        model.key(Key::RIGHT);
        CHECK(model.editing.caret() == 3);
        model.key(Key::BACKSPACE);
        CHECK(model.text() == "xy");
        CHECK(model.editing.caret() == 1);
    }

    SECTION("an emoji sequence joined by zero-width joiners") {
        Model model{family + "!"};
        model.key(Key::RIGHT);
        CHECK(model.editing.caret() == 8);
        model.key(Key::DELETE);
        CHECK(model.text() == family);
        model.key(Key::BACKSPACE);
        CHECK(model.text().empty());
    }

    SECTION("a letter and its combining mark") {
        Model model{eAcute + "t"};
        model.key(Key::RIGHT);
        CHECK(model.editing.caret() == 2);
        model.key(Key::LEFT);
        model.key(Key::DELETE);
        CHECK(model.text() == "t");
    }

    SECTION("the edges stay put") {
        Model model{"ab"};
        CHECK(model.key(Key::LEFT) == Edit::None);
        CHECK(model.key(Key::BACKSPACE) == Edit::None);
        model.key(Key::END);
        CHECK(model.key(Key::RIGHT) == Edit::None);
        CHECK(model.key(Key::DELETE) == Edit::None);
        CHECK(model.text() == "ab");
    }
}

TEST_CASE("TextEditing moves and erases by words", "[text][editing-model]") {
    Model model{"Hello brave  world"};

    SECTION("Ctrl+Right goes to the next word's end, past whitespace") {
        model.key(Key::RIGHT, ctrl);
        CHECK(model.editing.caret() == 5);
        model.key(Key::RIGHT, ctrl);
        CHECK(model.editing.caret() == 11);
        model.key(Key::RIGHT, ctrl);
        CHECK(model.editing.caret() == 18);
        CHECK(model.key(Key::RIGHT, ctrl) == Edit::None);
    }

    SECTION("Ctrl+Left goes to the previous word's start, past whitespace") {
        model.key(Key::END);
        model.key(Key::LEFT, ctrl);
        CHECK(model.editing.caret() == 13);
        model.key(Key::LEFT, ctrl);
        CHECK(model.editing.caret() == 6);
        // From inside a word, its own start
        model.editing.select(8, 8);
        model.key(Key::LEFT, ctrl);
        CHECK(model.editing.caret() == 6);
        model.key(Key::LEFT, ctrl);
        CHECK(model.editing.caret() == 0);
    }

    SECTION("Ctrl+Backspace and Ctrl+Delete erase as far") {
        model.editing.select(11, 11);
        model.key(Key::BACKSPACE, ctrl);
        CHECK(model.text() == "Hello   world");
        model.key(Key::DELETE, ctrl);
        CHECK(model.text() == "Hello ");
    }

    SECTION("a double click selects the word it lands on, not the space before it") {
        model.editing.selectAt(6, 2, false);
        CHECK(model.editing.selection() == std::make_pair(6u, 11u));
        model.editing.selectAt(8, 2, false);
        CHECK(model.editing.selection() == std::make_pair(6u, 11u));
        // Past the end, the last word
        model.editing.selectAt(18, 2, false);
        CHECK(model.editing.selection() == std::make_pair(13u, 18u));
    }
}

TEST_CASE("TextEditing selects with the mouse", "[text][editing-model]") {
    Model model{"Hello brave world. Second one."};

    SECTION("a press puts the caret, a drag selects from it") {
        CHECK(model.editing.selectAt(3, 1, false) == Edit::Caret);
        CHECK(model.editing.selection() == std::make_pair(3u, 3u));
        model.editing.dragTo(8);
        CHECK(model.editing.anchor() == 3);
        CHECK(model.editing.caret() == 8);
        model.editing.dragTo(1);
        CHECK(model.editing.selection() == std::make_pair(1u, 3u));
    }

    SECTION("Shift extends from the anchor") {
        model.editing.selectAt(3, 1, false);
        model.editing.selectAt(9, 1, true);
        CHECK(model.editing.anchor() == 3);
        CHECK(model.editing.caret() == 9);
    }

    SECTION("dragging after a double click keeps the word") {
        model.editing.selectAt(8, 2, false);
        model.editing.dragTo(14);
        CHECK(model.editing.selection() == std::make_pair(6u, 14u));
        model.editing.dragTo(2);
        CHECK(model.editing.selection() == std::make_pair(2u, 11u));
        CHECK(model.editing.caret() == 2);
    }

    SECTION("three clicks select the sentence, four everything") {
        model.editing.selectAt(3, 3, false);
        CHECK(model.editing.selection() == std::make_pair(0u, 19u));
        model.editing.selectAt(3, 4, false);
        CHECK(model.editing.selection() == std::make_pair(0u, 30u));
    }
}

TEST_CASE("TextEditing removes a selection first", "[text][editing-model]") {
    SECTION("select all then Delete with the caret at the end") {
        Model model{"hello"};
        model.key(Key::END);
        model.key(Key::A, ctrl);
        CHECK(model.editing.selection() == std::make_pair(0u, 5u));
        CHECK(model.key(Key::DELETE) == (Edit::Text | Edit::Caret));
        CHECK(model.text().empty());
    }

    SECTION("select all then Backspace with the caret at the start") {
        Model model{"hello"};
        model.key(Key::A, ctrl);
        model.key(Key::BACKSPACE);
        CHECK(model.text().empty());
    }

    SECTION("typing replaces it") {
        Model model{"hello"};
        model.editing.select(1, 4);
        model.type("ipp");
        CHECK(model.text() == "hippo");
        CHECK(model.editing.caret() == 4);
    }
}

TEST_CASE("TextEditing extends the selection with Shift", "[text][editing-model]") {
    Model model{"Hello world"};
    model.key(Key::RIGHT, shift);
    model.key(Key::RIGHT, shift);
    CHECK(model.editing.anchor() == 0);
    CHECK(model.editing.selection() == std::make_pair(0u, 2u));
    model.key(Key::RIGHT, ctrlShift);
    CHECK(model.editing.selection() == std::make_pair(0u, 5u));
    model.key(Key::END, shift);
    CHECK(model.editing.selection() == std::make_pair(0u, 11u));
    model.key(Key::LEFT, shift);
    CHECK(model.editing.selection() == std::make_pair(0u, 10u));
    // Back past the anchor, the other way
    model.key(Key::HOME);
    model.editing.select(5, 5);
    model.key(Key::LEFT, shift);
    model.key(Key::LEFT, shift);
    CHECK(model.editing.anchor() == 5);
    CHECK(model.editing.caret() == 3);
    CHECK(model.editing.selection() == std::make_pair(3u, 5u));
    // An arrow without Shift collapses it to its side
    model.key(Key::RIGHT);
    CHECK(model.editing.selection() == std::make_pair(5u, 5u));
    model.editing.select(2, 7);
    model.key(Key::LEFT);
    CHECK(model.editing.selection() == std::make_pair(2u, 2u));
}

TEST_CASE("TextEditing keeps one line on one line", "[text][editing-model]") {
    Model model{"Hello "};
    model.key(Key::END);
    model.clipboard.text = "big\nwide\r\nworld";
    model.key(Key::V, ctrl);
    CHECK(model.text() == "Hello big wide world");
    CHECK(model.editing.caret() == 20);
    CHECK(model.key(Key::ENTER) == Edit::None);

    SECTION("but not several") {
        Model lines{"", true};
        lines.clipboard.text = "a\nb";
        lines.key(Key::V, ctrl);
        lines.key(Key::ENTER);
        CHECK(lines.text() == "a\nb\n");
    }
}

TEST_CASE("TextEditing copies and cuts the selection", "[text][editing-model]") {
    Model model{"Hello wörld"};
    model.editing.select(6, 11);
    CHECK(model.key(Key::C, ctrl) == Edit::None);
    CHECK(model.clipboard.text == "wörld");
    model.key(Key::X, ctrl);
    CHECK(model.text() == "Hello ");
    // Without the modifier, nothing
    model.clipboard.text = "x";
    CHECK(model.key(Key::V) == Edit::None);
    CHECK(model.text() == "Hello ");
}

TEST_CASE("TextEditing moves between lines in the column it keeps", "[text][editing-model]") {
    // Columns: 6, 2, 8
    Model model{"abcdef\nab\nabcdefgh", true};
    model.editing.select(5, 5); // Column 5 on the first line

    model.key(Key::DOWN);
    CHECK(model.editing.caret() == 9); // The second line is shorter: its end
    model.key(Key::DOWN);
    CHECK(model.editing.caret() == 15); // Back in column 5
    model.key(Key::UP);
    model.key(Key::UP);
    CHECK(model.editing.caret() == 5);

    SECTION("past the first and last lines, the text's edges") {
        model.key(Key::UP);
        CHECK(model.editing.caret() == 0);
        model.key(Key::END, ctrl);
        model.key(Key::DOWN);
        CHECK(model.editing.caret() == 18);
    }

    SECTION("a horizontal move forgets the column") {
        model.key(Key::DOWN);
        model.key(Key::LEFT);
        model.key(Key::DOWN);
        CHECK(model.editing.caret() == 11);
    }

    SECTION("Home and End go to the line's edges, Ctrl to the text's") {
        model.key(Key::DOWN);
        model.key(Key::HOME);
        CHECK(model.editing.caret() == 7);
        model.key(Key::END);
        CHECK(model.editing.caret() == 9);
        model.key(Key::HOME, ctrl);
        CHECK(model.editing.caret() == 0);
    }

    SECTION("Shift+Down selects to the line below") {
        model.key(Key::DOWN, shift);
        CHECK(model.editing.selection() == std::make_pair(5u, 9u));
    }
}

TEST_CASE("Read-only TextEditing refuses edits but selects, moves and copies", "[text][editing-model]") {
    Model model{"Hello world"};
    model.editing.setReadOnly(true);
    CHECK(model.type("x") == Edit::None);
    CHECK(model.key(Key::DELETE) == Edit::None);
    CHECK(model.key(Key::RIGHT, shift) == Edit::Caret);
    CHECK(model.key(Key::BACKSPACE) == Edit::None);
    model.key(Key::A, ctrl);
    model.key(Key::C, ctrl);
    CHECK(model.clipboard.text == "Hello world");
    CHECK(model.key(Key::X, ctrl) == Edit::None);
    model.clipboard.text = "pasted";
    CHECK(model.key(Key::V, ctrl) == Edit::None);
    CHECK(model.text() == "Hello world");
    CHECK(model.editing.selection() == std::make_pair(0u, 11u));
}

TEST_CASE("TextEditing survives a move", "[text][editing-model]") {
    std::vector<TextEditing> models{};
    models.emplace_back().setText(icu::UnicodeString::fromUTF8("one two"));
    models.front().wordAt(1); // Its iterators made, on the first model's text
    models.emplace_back();      // Moves the first
    CHECK(models.front().wordAt(5) == std::make_pair(4u, 7u));
}
