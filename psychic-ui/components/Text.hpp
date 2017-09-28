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
        Text *setCaret(unsigned int caret);

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

        void subscribeToSelection();
        void unsubscribeFromSelection();
        void subscribeToEdition();
        void unsubscribeFromEdition();

        void updateDisplay();
        void handleKey(Key key);
    };
}
