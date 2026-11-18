#pragma once

#include <string>
#include <unicode/unistr.h>
#include "psychic-ui/Div.hpp"
#include "psychic-ui/text/TextGeometry.hpp"
#include "psychic-ui/text/TextLayout.hpp"

namespace psychic_ui {

    /**
     * @class Text
     *
     * Displays single or multiline text. A text field (`TextField`) drives it as its view: it sets the text, the
     * selection, whether the caret shows and, on one line, scrolls it to reveal the caret; the view paints them and
     * answers the editing model's geometry questions in its own coordinates (its border box, scrolled). The selection
     * is painted whenever there is one, in the inherited `selectionColor` and `selectionBackgroundColor` (none
     * while transparent), so rules decide how it looks while its field doesn't have the focus. It takes no input
     * itself: a text one can select and copy is a read-only field (`TextInput::readOnly`).
     *
     * Measured to its text: the widest line within the width it's given, and as many lines as that takes, each
     * the computed line height (glyphs may overflow a line height smaller than the font's, as in CSS).
     */
    class Text : public Div, public TextGeometry {
    public:
        /**
         * Construct a Text component, optionally initializing it with text
         * @param text
         */
        explicit Text(const std::string &text = "");

        /**
         * The text, in UTF-8, as it was set or edited
         */
        std::string text() const;

        /**
         * Set the text, the caret at its start
         */
        template<class Self>
        Self &text(this Self &self, const std::string &text) {
            static_cast<Text &>(self).updateText(text);
            return self;
        }

        bool multiline() const;

        /**
         * Whether the text wraps on several lines (or stays on one)
         */
        template<class Self>
        Self &multiline(this Self &self, detail::BoolArg multiline) {
            static_cast<Text &>(self).updateMultiline(multiline.value);
            return self;
        }

        /**
         * The selection, sorted: equal when nothing is selected
         */
        std::pair<unsigned int, unsigned int> selection() const;

        /**
         * Select from the anchor to the caret, which ends there. If both values are equal, then nothing is selected.
         */
        template<class Self>
        Self &selection(this Self &self, unsigned int anchor, unsigned int caret) {
            static_cast<Text &>(self).updateSelection(anchor, caret);
            return self;
        }

        bool caretVisible() const;

        /**
         * Whether the caret shows, for the field driving the view
         */
        template<class Self>
        Self &caretVisible(this Self &self, detail::BoolArg visible) {
            static_cast<Text &>(self).updateCaretVisible(visible.value);
            return self;
        }

        /**
         * How far one line is scrolled left, to show its caret
         */
        float offsetX() const;

        /**
         * Scroll one line horizontally just enough for the caret before the index to show, within the text (deleting
         * at the end scrolls it back); nothing on several lines. Needs the view laid out.
         */
        void reveal(unsigned int index);

        SkRect caretRect(unsigned int index) const override;
        unsigned int indexAt(float x, float y) const override;

    protected:
        icu::UnicodeString _text{};
        unsigned int       _anchor{0};
        unsigned int       _caret{0};
        bool               _multiline{false};
        float              _visibleWidth{0.0f}; // Where one line of static text is cut when it doesn't fit
        float              _offsetX{0.0f};
        bool               _caretVisible{false};

        /**
         * The text's layout (SkParagraph). Mutable: its queries lay the text out on demand.
         */
        mutable TextLayout _textLayout{};

        void updateText(const std::string &text);
        void updateMultiline(bool multiline);
        void updateCaretVisible(bool visible);
        void updateSelection(unsigned int anchor, unsigned int caret);

        void styleUpdated() override;
        Size measure(Constraint width, Constraint height) override;
        void layoutUpdated() override;
        void draw(SkCanvas *canvas) override;
    };
}
