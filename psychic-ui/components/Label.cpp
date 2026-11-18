#include <algorithm>
#include <unicode/unistr.h>
#include "Label.hpp"

namespace psychic_ui {

    Label::Label(const std::string &text) :
        TextBase::TextBase() {
        tag("Label");
        Label::updateText(text);
    }

    const std::string &Label::text() const {
        return _text;
    }

    void Label::updateText(const std::string &text) {
        if (_text != text) {
            _text = text;
            // We're not multi-line, so remove line returns
            std::replace(_text.begin(), _text.end(), '\n', ' ');
            _textLayout.setText(icu::UnicodeString::fromUTF8(_text));
            invalidate();
        }
    }

    Size Label::measure(Constraint width, Constraint height) {
        float maxContent = std::ceil(_textLayout.maxContentWidth());
        return Size{width.apply(maxContent), height.apply(linesHeight(1))};
    }

    void Label::layoutUpdated() {
        TextBase::layoutUpdated();
        _textLayout.layout(_paddedRect.width());
        // Cut after the last character that fits whole
        _visibleWidth = _textLayout.fittingWidth(_paddedRect.width());
    }

    void Label::draw(SkCanvas *canvas) {
        Div::draw(canvas);
        if (_text.empty()) {
            return;
        }
        bool cut = _visibleWidth < _textLayout.maxContentWidth();
        if (cut) {
            canvas->save();
            // Only horizontally: glyphs can reach above and below the line
            canvas->clipRect(SkRect::MakeLTRB(_paddedRect.fLeft, _paddedRect.fTop - _lineHeight, _paddedRect.fLeft + _visibleWidth, _paddedRect.fBottom + _lineHeight));
        }
        _textLayout.paint(canvas, _paddedRect.fLeft, _paddedRect.fTop);
        if (cut) {
            canvas->restore();
        }
    }
}
