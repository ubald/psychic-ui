#include "TextArea.hpp"

namespace psychic_ui {

    TextArea::TextArea(const std::string &text) :
        Component<TextAreaSkin>(),
        _text(text),
        _textDisplay(std::make_shared<Text>(_text)),
        _textScroller(std::make_shared<Scroller>(_textDisplay)) {
        setTag("TextArea");

        _focusEnabled = true;

        _textDisplay->setEditable(true);
        _textDisplay->setMultiline(true);
        _textDisplay->style()
                    ->set(minHeightPercent, 1.0f);

        _textDisplay->onChange(
            [this](std::string txt) {
                _text = txt;
                onChange(_text);
            }
        );

        _textDisplay->onCaret(
            [this](unsigned int /*caret*/) {
                int line = _textDisplay->getCaretLine();
                int yOver  = (line + 1) * _textDisplay->getLineHeight();
                int yUnder = line * _textDisplay->getLineHeight();

                std::cout << line << std::endl;

                if (yOver > -_textScroller->viewport()->scrollY() + _textScroller->viewport()->getHeight()) {
                    _textScroller->viewport()->setScrollY(-(yOver - _textScroller->viewport()->getHeight()));
                } else if (yUnder < -_textScroller->viewport()->scrollY()) {
                    _textScroller->viewport()->setScrollY(-(yUnder));
                }
            }
        );
    }

    std::shared_ptr<Text> TextArea::textDisplay() {
        return _textDisplay;
    };

    std::shared_ptr<Scroller> TextArea::textScroller() {
        return _textScroller;
    };

    const std::string &TextArea::getText() const {
        return _text;
    }

    TextArea *TextArea::setText(const std::string &text) {
        if (_text != text) {
            _text = text;
            _textDisplay->setText(_text);
        }
        return this;
    }

}
