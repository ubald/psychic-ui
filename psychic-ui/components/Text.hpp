#pragma once

#include <string>
#include <unicode/unistr.h>
#include "psychic-ui/TextBase.hpp"
#include "psychic-ui/Div.hpp"
#include "psychic-ui/text/TextEditing.hpp"

namespace psychic_ui {

    /**
     * @class Text
     *
     * Displays single or multiline text that is optionally selectable and/or editable, through a TextEditing.
     * It answers the editing model's geometry questions in its own coordinates (inside its padding and border).
     */
    class Text : public TextBase, public TextEditing::Geometry {
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

        SkRect caretRect(unsigned int index) const override;
        unsigned int indexAt(float x, float y) const override;

        Signal<unsigned int, unsigned int> selectionChanged{};
        Signal<unsigned int>               caretMoved{};
        Signal<std::string>                changed{};

        PSYCHIC_UI_EVENT(onSelectionChange, selectionChanged)
        PSYCHIC_UI_EVENT(onCaretMove, caretMoved)
        PSYCHIC_UI_EVENT(onChange, changed)

    protected:

        bool        _selectable{true};
        bool        _editable{false};
        TextEditing _editing{};

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

        void updateText(const std::string &text) override;
        void updateSelectable(bool selectable);
        void updateEditable(bool editable);
        void updateMultiline(bool multiline);
        void updateSelection(unsigned int anchor, unsigned int caret);
        void moveCaret(unsigned int caret);

        void subscribeToSelection();
        void subscribeToEdition();

        void removedFromRender() override;
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
