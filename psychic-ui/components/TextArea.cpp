#include "TextArea.hpp"

namespace psychic_ui {

    TextArea::TextArea(const std::string &text) :
        Component<TextAreaSkin>(),
        _text(text),
        _textDisplay(std::make_shared<Text>(_text)),
        _textScroller(std::make_shared<Scroller>(_textDisplay)) {
        tag("TextArea");

        _focusEnabled = true;

        _textDisplay->editable(true);
        _textDisplay->multiline(true);
        _textDisplay->minHeight(100_pct);

        _textDisplay->onChange(
            [this](std::string txt) {
                _text = txt;
                changed.emit(_text);
            }
        );

        _textDisplay->onCaretMove(
            [this](unsigned int /*caret*/) {
                int line = _textDisplay->getCaretLine();
                int yOver  = (line + 1) * _textDisplay->getLineHeight();
                int yUnder = line * _textDisplay->getLineHeight();

                if (yOver > -_textScroller->viewport()->scrollY() + _textScroller->viewport()->getHeight()) {
                    _textScroller->viewport()->scrollY(-(yOver - _textScroller->viewport()->getHeight()));
                } else if (yUnder < -_textScroller->viewport()->scrollY()) {
                    _textScroller->viewport()->scrollY(-(yUnder));
                }
            }
        );
    }

    Div *TextArea::focusDelegate() {
        return _textDisplay.get();
    }

    std::shared_ptr<Text> TextArea::textDisplay() {
        return _textDisplay;
    };

    std::shared_ptr<Scroller> TextArea::textScroller() {
        return _textScroller;
    };

    const std::string &TextArea::text() const {
        return _text;
    }

    void TextArea::updateText(const std::string &text) {
        if (_text != text) {
            _text = text;
            _textDisplay->text(_text);
        }
    }

}
