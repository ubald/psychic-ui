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
     * Displays single or multiline text that is optionally selectable.
     */
    class Text : public TextBase {
    public:
        explicit Text(const std::string &text = "");

        std::string text() const;
        void setText(const std::string &text) override;

        bool selectable() const;
        Text *setSelectable(bool lcdRender);

        bool editable() const;
        Text *setEditable(bool editable);

        bool multiline() const;
        Text *setMultiline(bool multiline);

        std::pair<unsigned int, unsigned int> getSelection() const;
        Text *setSelection(std::pair<unsigned int, unsigned int> selection);
        unsigned int getCaret() const;

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

        void styleUpdated() override;
        YGSize measure(float width, YGMeasureMode widthMode, float height, YGMeasureMode heightMode) override;
        void layoutUpdated() override;
        void draw(SkCanvas *canvas) override;

        Signal<unsigned int, unsigned int> onSelection{};
        Signal<unsigned int>               onCaret{};
        Signal<std::string>                onChange{};

    protected:
        using BlobPtr = std::unique_ptr<SkTextBlob, std::function<void(SkTextBlob *)>>;

        bool          _selectable{true};
        bool          _editable{false};
        bool          _multiline{true};
        unsigned int  _selectBegin{0};
        unsigned int  _selectEnd{0};
        unsigned int  _caret{0};
        unsigned int  _targetXPos{0};
        UnicodeString _text{};
        TextBox       _textBox{};
        BlobPtr       _blob{nullptr};
        SkPaint       _selectionPaint{};
        SkPaint       _selectionBackgroundPaint{};

        Div::MouseSlot _onMouseMove{nullptr};
        Div::MouseSlot _onMouseDown{nullptr};
        Div::MouseSlot _onMouseUp{nullptr};
        Div::KeySlot   _onKeyDown{nullptr};
        Div::KeySlot   _onKeyRepeat{nullptr};
        Div::CharSlot  _onCharacter{nullptr};
        Div::FocusSlot _onFocus{nullptr};
        Div::FocusSlot _onBlur{nullptr};

        void subscribeToSelection();
        void unsubscribeFromSelection();
        void subscribeToEdition();
        void unsubscribeFromEdition();

        void updateDisplay();
        void handleKey(Key key, Mod mod);
    };
}
