#pragma once

#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <unicode/brkiter.h>
#include <unicode/unistr.h>
#include "include/core/SkRect.h"
#include "psychic-ui/psychic-ui.hpp"

namespace psychic_ui {

    /**
     * @class TextEditing
     *
     * The editing model of a text: the text itself, an anchor and a caret (the selection is what lies between them),
     * the column that line moves keep, and what keys, typing and the mouse do to them. No Div: what shows the text
     * answers the geometry questions (`Geometry`), and the clipboard is given (`Clipboard`).
     *
     * Every index is in UTF-16 code units, like icu::UnicodeString's, and moves step over whole grapheme clusters
     * (an emoji, a letter and its combining marks). Nothing is emitted: every operation returns what it changed
     * (`Edit`), so that what owns the model tells the world once its own state is consistent.
     *
     * Read-only text refuses edits but keeps the selection, the caret's moves and copying.
     */
    class TextEditing {
    public:
        /**
         * What an operation changed, as a bitmask (`Edit::Caret | Edit::Text`).
         * Every combination is an enumerator, so that `|` never makes a value the type doesn't name.
         */
        enum class Edit : unsigned {
            None         = 0,
            Caret        = 1u << 0, // The anchor or the caret
            Text         = 1u << 1,
            TextAndCaret = Caret | Text
        };

        enum class Direction { Backward, Forward };

        /**
         * How far a move or an erase goes
         */
        enum class Unit {
            Grapheme, // A user-perceived character
            Word,     // The previous word's start, or the next word's end, past whitespace
            Line,     // The line above or below, in the column kept (needs the geometry)
            LineEdge, // The start or the end of the caret's line (needs the geometry)
            All       // The start or the end of the text
        };

        /**
         * What shows the text answers these, in its own coordinates
         */
        class Geometry {
        public:
            virtual ~Geometry() = default;

            /**
             * The caret before the character at the index: a zero-width rectangle across its line
             */
            virtual SkRect caretRect(unsigned int index) const = 0;

            /**
             * The caret index closest to a point
             */
            virtual unsigned int indexAt(float x, float y) const = 0;
        };

        /**
         * Where copied text goes and pasted text comes from, in UTF-8
         */
        class Clipboard {
        public:
            virtual ~Clipboard() = default;
            virtual std::string clipboardText() const = 0;
            virtual void setClipboardText(const std::string &text) = 0;
        };

        TextEditing();
        ~TextEditing();
        TextEditing(TextEditing &&) noexcept;
        TextEditing &operator=(TextEditing &&) noexcept;

        const icu::UnicodeString &text() const;

        /**
         * The text in UTF-8
         */
        std::string utf8() const;

        /**
         * Replace the text, the caret at its start
         */
        Edit setText(const icu::UnicodeString &text);

        unsigned int anchor() const;
        unsigned int caret() const;

        /**
         * The selection, sorted: equal when nothing is selected
         */
        std::pair<unsigned int, unsigned int> selection() const;
        bool hasSelection() const;
        std::u16string selectedText() const;

        /**
         * Select from the anchor to the caret (clamped to the text), forgetting the column kept
         */
        Edit select(unsigned int anchor, unsigned int caret);

        /**
         * Whether the text keeps its line returns: on one line, inserted line returns become spaces and Enter does
         * nothing
         */
        bool multiline() const;
        void setMultiline(bool multiline);

        /**
         * Whether edits are refused
         */
        bool readOnly() const;
        void setReadOnly(bool readOnly);

        /**
         * Replace the selection with the string, the caret after it
         */
        Edit insert(const icu::UnicodeString &str);

        /**
         * Remove the selection, or else from the caret to where a move would go
         */
        Edit erase(Direction direction, Unit unit, const Geometry *geometry = nullptr);

        /**
         * Move the caret, extending the selection from the anchor or collapsing it. Without extending, a grapheme
         * move with a selection collapses it to its side. Line and line edge moves need the geometry.
         */
        Edit move(Direction direction, Unit unit, bool extend, const Geometry *geometry = nullptr);

        /**
         * A press at the index: the caret there (1 click, or extending the selection), the word (2), the sentence
         * (3) or everything (4 and more)
         */
        Edit selectAt(unsigned int index, unsigned int clickCount, bool extend);

        /**
         * Drag from the last press to the index: the selection grows from what the press selected
         */
        Edit dragTo(unsigned int index);

        /**
         * What a key does: moves (Shift extends), erasing, Enter on several lines, select all, copy, cut and paste
         * (Ctrl or Super)
         * @return What changed, `Edit::None` for a key it has no use for
         */
        Edit handleKey(Key key, Mod mod, const Geometry &geometry, Clipboard &clipboard);

        /**
         * Word and sentence boundaries around an index, for double and triple clicks
         */
        std::pair<unsigned int, unsigned int> wordAt(unsigned int index) const;
        std::pair<unsigned int, unsigned int> sentenceAt(unsigned int index) const;

    private:
        icu::UnicodeString           _text{};
        unsigned int                 _anchor{0};
        unsigned int                 _caret{0};
        std::optional<float>         _goalX{}; // The column line moves keep
        std::pair<unsigned, unsigned> _pressed{0, 0}; // What the last press selected, which a drag grows from
        bool                         _multiline{false};
        bool                         _readOnly{false};

        // Made on first use, so that text never edited costs no ICU iterator; pointed at the text before each use
        mutable std::unique_ptr<icu::BreakIterator> _graphemes{nullptr};
        mutable std::unique_ptr<icu::BreakIterator> _words{nullptr};
        mutable std::unique_ptr<icu::BreakIterator> _sentences{nullptr};

        icu::BreakIterator &graphemes() const;
        icu::BreakIterator &words() const;
        icu::BreakIterator &sentences() const;

        unsigned int length() const;
        unsigned int previousGrapheme(unsigned int index) const;
        unsigned int nextGrapheme(unsigned int index) const;
        unsigned int previousWordStart(unsigned int index) const;
        unsigned int nextWordEnd(unsigned int index) const;
        std::pair<unsigned int, unsigned int> segmentAt(icu::BreakIterator &iterator, unsigned int index) const;
        bool isWhitespace(unsigned int begin, unsigned int end) const;

        /**
         * Where a move from the caret goes
         */
        unsigned int target(Direction direction, Unit unit, const Geometry *geometry);

        /**
         * Set the anchor and the caret, saying whether they changed
         */
        Edit place(unsigned int anchor, unsigned int caret);

        /**
         * Replace a range with a string, the caret after it
         */
        Edit replace(unsigned int begin, unsigned int end, const icu::UnicodeString &str);
    };

    constexpr TextEditing::Edit operator|(TextEditing::Edit a, TextEditing::Edit b) {
        return static_cast<TextEditing::Edit>(static_cast<unsigned>(a) | static_cast<unsigned>(b));
    }

    constexpr TextEditing::Edit &operator|=(TextEditing::Edit &a, TextEditing::Edit b) {
        return a = a | b;
    }

    /**
     * Whether an edit changed anything of what is asked (`has(edit, Edit::Text)`)
     */
    constexpr bool has(TextEditing::Edit edit, TextEditing::Edit what) {
        return (static_cast<unsigned>(edit) & static_cast<unsigned>(what)) != 0;
    }
}
