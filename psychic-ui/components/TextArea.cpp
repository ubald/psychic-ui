#include <cmath>
#include <utility>
#include "TextArea.hpp"

namespace psychic_ui {

    TextArea::TextArea(std::string text) :
        Component<TextAreaSkin>(),
        _text(std::move(text)),
        _textDisplay(std::make_shared<Text>(_text)),
        _textScroller(std::make_shared<Scroller>(_textDisplay)) {
        tag("TextArea");

        _focusEnabled = true;

        _textDisplay->editable(true);
        _textDisplay->multiline(true);
        _textDisplay->minHeight(100_pct);

        _textDisplay->onChange(
            [this](std::string txt) {
                _text = std::move(txt);
                changed.emit(_text);
            }
        );

        _textDisplay->onCaretMove(
            [this](unsigned int caret) {
                const SkRect rect   = _textDisplay->caretRect(caret);
                const int    yOver  = static_cast<int>(std::ceil(rect.fBottom));
                const int    yUnder = static_cast<int>(rect.fTop);

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
