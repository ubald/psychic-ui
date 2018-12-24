#include <algorithm>
#include "Label.hpp"

namespace psychic_ui {

    Label::Label(const std::string &text) :
        TextBase::TextBase() {
        setTag("Label");
        setText(text);
    }

    const std::string &Label::text() const {
        return _text;
    }

    Label *Label::setText(const std::string &text) {
        if (_text != text) {
            _text = text;
            // We're not multi-line, so remove line returns
            std::replace(_text.begin(), _text.end(), '\n', ' ');
            invalidate();
        }
        return this;
    }

    void Label::styleUpdated() {
        TextBase::styleUpdated();
        SkFontMetrics metrics{};
        _textPaint.getFontMetrics(&metrics);
        _yOffset = -metrics.fAscent;
    }

    YGSize Label::measure(float width, YGMeasureMode widthMode, float /*height*/, YGMeasureMode /*heightMode*/) {
        YGSize size{0.0f, _lineHeight};

        if (_text.empty()) {
            return size;
        }

        if (widthMode == YGMeasureModeExactly) {
            size.width = width;
        } else {
            size.width = std::ceil(_textPaint.measureText(_text.c_str(), _text.size()));
            if (widthMode == YGMeasureModeAtMost) {
                size.width = std::min(size.width, width);
            }
        }

        return size;
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
