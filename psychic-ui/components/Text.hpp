#pragma once

#include <string>
#include <SkTextBlob.h>
#include <unicode/unistr.h>
#include "psychic-ui/utils/TextBox.hpp"
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

        /**
         * Set the component's text
         * @param text
         * @return Pointer to Text for chaining
         */
        Text *setText(const std::string &text) override;

        /**
         * Get whether the text is selectable or not
         * @return
         */
        bool selectable() const;

        /**
         * Set whether the text is selectable or not
         * @param selectable
         * @return Pointer to Text for chaining
         */
        Text *setSelectable(bool selectable);

        /**
         * Get whether the text is editable or not
         * @return
         */
        bool editable() const;

        /**
         * Set whether the text is editable or not
         * @param editable
         * @return Pointer to Text for chaining
         */
        Text *setEditable(bool editable);

        /**
         * Get whether the text is multiline or not
         * @return
         */
        bool multiline() const;

        /**
         * Set whether the text is multiline or not
         * @param multiline
         * @return Pointer to Text for chaining
         */
        Text *setMultiline(bool multiline);

        /**
         * Get the selection start and end index.
         * If both values are equal, then nothing is selected.
         *
         * @return Pair of start and end positions of the selection
         */
        std::pair<unsigned int, unsigned int> getSelection() const;

        /**
         * Set the selection.
         * If both values are equal, then nothing is selected.
         *
         * @param selectionBegin
         * @param selectionEnd
         * @return Pointer to Text for chaining
         */
        Text *setSelection(unsigned int selectionBegin, unsigned int selectionEnd);

        /**
         * Get the caret position
         * @return
         */
        unsigned int getCaret() const;

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

        /**
         * Set the caret position in the text
         * @param caret Position of the caret
         * @param saveX Used internally, saves where the caret is horizontally
         *              in order to restore it horizontally in case navigating
         *              in the text with the keyboard makes us pass by a line
         *              that is shorter, we still want to navigate up or down
         *              in the same column.
         * @return Pointer to this, for call chaining
         */
        Text *setCaret(unsigned int caret, bool saveX = true);


        Signal<unsigned int, unsigned int> onSelection{};
        Signal<unsigned int>               onCaret{};
        Signal<std::string>                onChange{};

    protected:
        using BlobPtr = std::unique_ptr<SkTextBlob, std::function<void(SkTextBlob *)>>;

        bool               _selectable{true};
        bool               _editable{false};
        bool               _multiline{false};
        unsigned int       _selectBegin{0};
        unsigned int       _selectEnd{0};
        unsigned int       _caret{0};
        unsigned int       _targetXPos{0};
        icu::UnicodeString _text{};
        TextBox            _textBox{};
        BlobPtr            _blob{nullptr};
        SkPaint            _selectionPaint{};
        SkPaint            _selectionBackgroundPaint{};

        /**
         * Whether we're waiting on layout validation to sent the caret signal
         */
        bool _pendingCaretSignal{false};

        Div::MouseMoveSlot        _onMouseMove{nullptr};
        Div::MouseSlot            _onMouseDown{nullptr};
        Div::MouseSlot            _onMouseUp{nullptr};
        Div::MouseDoubleClickSlot _onDoubleClick{nullptr};
        Div::KeySlot              _onKeyDown{nullptr};
        Div::KeySlot              _onKeyRepeat{nullptr};
        Div::CharSlot             _onCharacter{nullptr};
        Div::FocusSlot            _onFocus{nullptr};
        Div::FocusSlot            _onBlur{nullptr};

        void subscribeToSelection();
        void unsubscribeFromSelection();
        void subscribeToEdition();
        void unsubscribeFromEdition();

        void styleUpdated() override;
        YGSize measure(float width, YGMeasureMode widthMode, float height, YGMeasureMode heightMode) override;
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
    };
}
