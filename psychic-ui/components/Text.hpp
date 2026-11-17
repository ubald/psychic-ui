#pragma once

#include <string>
#include <unicode/unistr.h>
#include "psychic-ui/TextBase.hpp"
#include "psychic-ui/Div.hpp"

namespace psychic_ui {

    /**
     * @class Text
     *
     * Displays single or multiline text that is optionally selectable and/or editable.
     */
    class Text : public TextBase {
    public:
        /**
         * Construct a Text component, optionally initializing it with text
         * @param text
         */
        explicit Text(const std::string &text = "");

        /**
         * Get the component's text
         * @return
         */
        std::string text() const;

        // The setter, which the getter above would hide
        using TextBase::text;

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
         * Set the selection. If both values are equal, then nothing is selected.
         */
        template<class Self>
        Self &selection(this Self &self, unsigned int selectionBegin, unsigned int selectionEnd) {
            static_cast<Text &>(self).updateSelection(selectionBegin, selectionEnd);
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

        /**
         * Get the line the caret is on
         * @return
         */
        unsigned int getCaretLine() const;

        /**
         * Get the Y coordinate of the line.
         * @param line Line number
         * @return Y coordinate, line * line height
         */
        unsigned int getLineY(unsigned int line) const;


        Signal<unsigned int, unsigned int> selectionChanged{};
        Signal<unsigned int>               caretMoved{};
        Signal<std::string>                changed{};

        PSYCHIC_UI_EVENT(onSelectionChange, selectionChanged)
        PSYCHIC_UI_EVENT(onCaretMove, caretMoved)
        PSYCHIC_UI_EVENT(onChange, changed)

    protected:

        bool               _selectable{true};
        bool               _editable{false};
        bool               _multiline{false};
        unsigned int       _selectBegin{0};
        unsigned int       _selectEnd{0};
        unsigned int       _caret{0};
        float              _targetXPos{0.0f};
        icu::UnicodeString _text{};
        SkPaint            _selectionBackgroundPaint{};

        /**
         * Whether we're waiting on layout validation to sent the caret signal
         */
        bool _pendingCaretSignal{false};

        ScopedSlot                _onMouseMove{}; // On the window, only while selecting
        std::vector<ScopedSlot>   _selectionSlots{}; // Ours while selectable
        std::vector<ScopedSlot>   _editionSlots{}; // Ours while editable

        /**
         * In touch mode, tell the window where the caret's line is, for the on-screen keyboard
         */
        void updateTextInputArea();

        void updateText(const std::string &text) override;
        void updateSelectable(bool selectable);
        void updateEditable(bool editable);
        void updateMultiline(bool multiline);
        void updateSelection(unsigned int selectionBegin, unsigned int selectionEnd);

        /**
         * Set the caret position in the text
         * @param caret Position of the caret
         * @param saveX Used internally, saves where the caret is horizontally
         *              in order to restore it horizontally in case navigating
         *              in the text with the keyboard makes us pass by a line
         *              that is shorter, we still want to navigate up or down
         *              in the same column.
         */
        void moveCaret(unsigned int caret, bool saveX = true);

        void subscribeToSelection();
        void subscribeToEdition();

        void removedFromRender() override;
        void styleUpdated() override;
        Size measure(Constraint width, Constraint height) override;
        void layoutUpdated() override;
        void draw(SkCanvas *canvas) override;

        /**
         * Call when text has changed in another manner than using `setText`.
         * Since we're referencing the string we habe to call this method whenever
         * the text inside string changes.
         */
        void textChanged();

        /**
         * Called when text was edited, as a shortcut to both
         * `textChanged` and `setCaret` since they have to be called
         * in a particular order and we don't want to repeat both calls.
         *
         * @param caret Position the caret should be at after the text was edited
         */
        void textEdited(unsigned int caret);

        void handleKey(Key key, Mod mod);

        /**
         * The caret index at a point in the Div's own coordinates
         */
        unsigned int indexAt(int x, int y) const;
    };
}
