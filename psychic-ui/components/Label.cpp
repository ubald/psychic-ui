#include <algorithm>
#include "Label.hpp"
#include "psychic-ui/utils/TextBox.hpp"

namespace psychic_ui {

    Label::Label(const std::string &text) :
        TextBase::TextBase() {
        tag("Label");
        updateText(text);
    }

    const std::string &Label::text() const {
        return _text;
    }

    void Label::updateText(const std::string &text) {
        if (_text != text) {
            _text = text;
            // We're not multi-line, so remove line returns
            std::replace(_text.begin(), _text.end(), '\n', ' ');
            invalidate();
        }
    }

    void Label::styleUpdated() {
        TextBase::styleUpdated();
        SkFontMetrics metrics{};
        _font.getMetrics(&metrics);
        _yOffset = -metrics.fAscent;
    }

    Size Label::measure(Constraint width, Constraint height) {
        float maxContent = std::ceil(_font.measureText(_text.c_str(), _text.size(), SkTextEncoding::kUTF8));
        return Size{width.apply(maxContent), height.apply(linesHeight(1))};
    }

    void Label::layoutUpdated() {
        TextBase::layoutUpdated();
        _drawText = _text.substr(0, breakText(_font, _text.c_str(), _text.size(), _paddedRect.width()));
    }

    void Label::draw(SkCanvas *canvas) {
        Div::draw(canvas);
        if (!_text.empty()) {
            canvas->drawSimpleText(
                _drawText.c_str(), _drawText.size(), SkTextEncoding::kUTF8,
                _paddedRect.fLeft, _paddedRect.fTop + _yOffset, _font, _textPaint
            );
        }
    }
}
