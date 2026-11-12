#include <algorithm>
#include "Label.hpp"

namespace psychic_ui {

    Label::Label(const std::string &text) :
        TextBase::TextBase() {
        setTag("Label");
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
        _textPaint.getFontMetrics(&metrics);
        _yOffset = -metrics.fAscent;
    }

    YGSize Label::measure(float width, YGMeasureMode widthMode, float height, YGMeasureMode heightMode) {
        float maxContent = std::ceil(_textPaint.measureText(_text.c_str(), _text.size()));
        return YGSize{fit(maxContent, width, widthMode), fit(linesHeight(1), height, heightMode)};
    }

    void Label::layoutUpdated() {
        TextBase::layoutUpdated();
        // TODO: Fix deprecated thing
        SkFont font = SkFont::LEGACY_ExtractFromPaint(_textPaint);
        _drawText = _text.substr(0, font.breakText(_text.c_str(), _text.size(), SkTextEncoding::kUTF8, _paddedRect.width()));
    }

    void Label::draw(SkCanvas *canvas) {
        Div::draw(canvas);
        if (!_text.empty()) {
            canvas->drawText(_drawText.c_str(), _drawText.size(), _paddedRect.fLeft, _paddedRect.fTop + _yOffset, _textPaint);
        }
    }
}
