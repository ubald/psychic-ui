#pragma once

#include <string>
#include <unicode/unistr.h>
#include "psychic-ui/Div.hpp"
#include "psychic-ui/text/TextEditing.hpp"
#include "psychic-ui/text/TextLayout.hpp"

namespace psychic_ui {

    /**
     * @class Text
     *
     * Displays single or multiline text. A text field (`TextField`) drives it as its view: it sets the text, the
     * selection, whether the caret shows and, on one line, scrolls it to reveal the caret; the view paints them and
     * answers the editing model's geometry questions in its own coordinates (its border box, scrolled). The selection
     * is painted whenever there is one, in the inherited `selectionColor` and `selectionBackgroundColor` (none
     * while transparent), so rules decide how it looks while its field doesn't have the focus.
     *
     * It can still edit itself (`editable`, `selectable`), which only `TextArea` and the demo use until they move to
     * a field.
     *
     * Measured to its text: the widest line within the width it's given, and as many lines as that takes, each
     * the computed line height (glyphs may overflow a line height smaller than the font's, as in CSS).
     */
    class Text : public Div, public TextEditing::Geometry {
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

        bool selectable() const;

        /**
         * Whether the text can be selected with the mouse
         */
        template<class Self>
        Self &selectable(this Self &self, detail::BoolArg selectable) {
            static_cast<Text &>(self).updateSelectable(selectable.value);
            return self;
        }

        bool editable() const;

        /**
         * Whether the text can be edited when focused
         */
        template<class Self>
        Self &editable(this Self &self, detail::BoolArg editable) {
            static_cast<Text &>(self).updateEditable(editable.value);
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
         * Get the selection start and end index.
         * If both values are equal, then nothing is selected.
         *
         * @return Pair of start and end positions of the selection
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

        /**
         * Get the caret position
         * @return
         */
        unsigned int caret() const;

        /**
         * Move the caret, which also empties the selection
         */
        template<class Self>
        Self &caret(this Self &self, unsigned int caret) {
            static_cast<Text &>(self).moveCaret(caret);
            return self;
        }

        bool caretVisible() const;

        /**
         * Whether the caret shows, for the field driving the view (an editable text shows it while focused)
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

        Signal<unsigned int, unsigned int> selectionChanged{};
        Signal<unsigned int>               caretMoved{};
        Signal<std::string>                changed{};

        PSYCHIC_UI_EVENT(onSelectionChange, selectionChanged)
        PSYCHIC_UI_EVENT(onCaretMove, caretMoved)
        PSYCHIC_UI_EVENT(onChange, changed)

    protected:

        float       _visibleWidth{0.0f}; // Where one line of static text is cut when it doesn't fit
        float       _offsetX{0.0f};
        bool        _caretVisible{false};
        bool        _selectable{true};
        bool        _editable{false};
        TextEditing _editing{};

        /**
         * The text's layout (SkParagraph). Mutable: its queries lay the text out on demand.
         */
        mutable TextLayout _textLayout{};

        /**
         * Whether we're waiting on layout validation to sent the caret signal
         */
        bool _pendingCaretSignal{false};

        ScopedSlot                _onMouseMove{}; // On the window, only while selecting
        std::vector<ScopedSlot>   _selectionSlots{}; // Ours while selectable
        std::vector<ScopedSlot>   _editionSlots{}; // Ours while editable

        /**
         * Tell the window where the caret's line is, for the on-screen keyboard and the input method's candidates
         */
        void updateTextInputArea();

        void updateText(const std::string &text);
        void updateSelectable(bool selectable);
        void updateEditable(bool editable);
        void updateMultiline(bool multiline);
        void updateCaretVisible(bool visible);
        void updateSelection(unsigned int anchor, unsigned int caret);
        void moveCaret(unsigned int caret);

        void subscribeToSelection();
        void subscribeToEdition();

        void removedFromRender() override;
        void styleUpdated() override;
        Size measure(Constraint width, Constraint height) override;
        void layoutUpdated() override;
        void draw(SkCanvas *canvas) override;

        /**
         * Show and tell what an edit changed: the text laid out again and `changed`, then the caret (`caretMoved`,
         * once laid out, so that its position is right) and `selectionChanged`
         */
        void apply(TextEditing::Edit edit);

        void handleKey(Key key, Mod mod);
    };
}
