#include <algorithm>
#include <cmath>
#include <cstddef>
#include <limits>
#include <string>
#include <utility>
#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "Text.hpp"
#include "psychic-ui/Div.hpp"
#include "psychic-ui/Measure.hpp"
#include "psychic-ui/style/Properties.hpp"
#include "psychic-ui/style/Values.hpp"
#include "psychic-ui/text/TextLayout.hpp"

namespace psychic_ui {

    Text::Text(const std::string &text) :
        Div::Div() {
        tag("Text");

        _defaultStyle.shrink(0);
        setMeasurable();

        #ifdef DEBUG_LAYOUT
        dashed = true;
        #endif

        Text::updateText(text);
    }

    std::string Text::text() const {
        std::string utf8{};
        _text.toUTF8String(utf8);
        return utf8;
    }

    void Text::updateText(const std::string &text) {
        // The caret goes back to the start, the text is only laid out again when it changed
        _anchor = 0;
        _caret  = 0;
        const icu::UnicodeString text16 = icu::UnicodeString::fromUTF8(text);
        if (text16 != _text) {
            _text = text16;
            _textLayout.setText(_text);
            invalidate();
        }
    }

    bool Text::multiline() const {
        return _multiline;
    }

    void Text::updateMultiline(const bool multiline) {
        if (_multiline != multiline) {
            _multiline = multiline;
            _textLayout.setMultiline(multiline);
            invalidate();
        }
    }

    bool Text::caretVisible() const {
        return _caretVisible;
    }

    void Text::updateCaretVisible(const bool visible) {
        _caretVisible = visible;
    }

    float Text::offsetX() const {
        return _offsetX;
    }

    void Text::reveal(const unsigned int index) {
        if (_multiline) {
            _offsetX = 0.0f;
            return;
        }
        const float width  = _paddedRect.width();
        const float x      = _textLayout.caretRect(index).fLeft;
        float       offset = _offsetX;
        if (x - offset > width - 1.0f) {
            offset = x - width + 1.0f; // The caret's bar is 2 px wide, centered on its x
        }
        if (x - offset < 1.0f) {
            offset = x - 1.0f;
        }
        _offsetX = std::clamp(offset, 0.0f, std::max(0.0f, std::ceil(_textLayout.maxContentWidth()) + 1.0f - width));
    }

    std::pair<unsigned int, unsigned int> Text::selection() const {
        return std::minmax(_anchor, _caret);
    }

    void Text::updateSelection(const unsigned int anchor, const unsigned int caret) {
        const auto length = static_cast<unsigned int>(_text.length());
        _anchor = std::min(anchor, length);
        _caret  = std::min(caret, length);
    }

    SkRect Text::caretRect(unsigned int index) const {
        // The text starts inside the padding and border, scrolled
        return _textLayout.caretRect(index).makeOffset(_paddedRect.fLeft - _rect.fLeft - _offsetX, _paddedRect.fTop - _rect.fTop);
    }

    unsigned int Text::indexAt(float x, float y) const {
        return _textLayout.indexAt(x - (_paddedRect.fLeft - _rect.fLeft - _offsetX), y - (_paddedRect.fTop - _rect.fTop));
    }

    void Text::styleUpdated() {
        Div::styleUpdated();

        const float fontSize = _computedStyle.fontSize();
        const bool  changed  = _textLayout.setStyle(TextLayoutStyle{
            .fonts         = styleManager()->fontCollection(),
            .fontFamily    = _computedStyle.fontFamily().str(),
            .fontSize      = fontSize,
            // Auto is 1.5 times the font size, a percent is of the font size
            .lineHeight    = _computedStyle.lineHeight().resolve(fontSize, fontSize * 1.5f),
            .letterSpacing = _computedStyle.letterSpacing(),
            .smoothing     = _computedStyle.fontSmoothing(),
            .align         = _computedStyle.textAlign(),
            .overflow      = _computedStyle.textOverflow()
        });
        // Yoga caches measures: re-measure when anything changing the text metrics changes. Colors aren't part of
        // the layout: they are given when painting, so a color transition lays nothing out again
        if (changed) {
            invalidate();
        }
    }

    Size Text::measure(Constraint width, Constraint height) {
        // Pure: the layout caches measures and calls this with various constraints,
        // so the layout used for painting (at the box's width) is left as it is.
        const float contentWidth = width.apply(std::ceil(_textLayout.maxContentWidth()));
        auto  lines        = std::max<size_t>(1, _textLayout.lineCountAt(contentWidth));
        return Size{.width = contentWidth, .height = height.apply(static_cast<float>(lines) * _textLayout.style().lineHeight)};
    }

    void Text::layoutUpdated() {
        Div::layoutUpdated();
        _textLayout.layout(_paddedRect.width());
        // One line of static text that doesn't fit is cut after the last character that fits whole (textOverflow's
        // Clip; Ellipsis is the layout's own)
        const bool cut = !_multiline && _computedStyle.textOverflow() == TextOverflow::Clip;
        _visibleWidth  = cut ? _textLayout.fittingWidth(_paddedRect.width()) : std::numeric_limits<float>::infinity();
    }

    void Text::draw(SkCanvas *canvas) {
        Div::draw(canvas);
        const float left = _paddedRect.fLeft - _offsetX;
        // Scrolled, the line is clipped to the box (the caret's bar may reach a pixel past it); else one line of
        // static text that doesn't fit is cut after its last whole character
        const bool  scrolled = _offsetX != 0.0f;
        const bool  cut      = !scrolled && _visibleWidth < _textLayout.maxContentWidth();
        if (scrolled || cut) {
            canvas->save();
            // Only horizontally: glyphs can reach above and below the line
            const float lineHeight = _textLayout.style().lineHeight;
            const float right      = scrolled ? _paddedRect.fRight + 1.0f : _paddedRect.fLeft + _visibleWidth;
            canvas->clipRect(SkRect::MakeLTRB(_paddedRect.fLeft - (scrolled ? 1.0f : 0.0f), _paddedRect.fTop - lineHeight, right, _paddedRect.fBottom + lineHeight));
        }
        if (!_text.isEmpty()) {
            _textLayout.paint(canvas, left, _paddedRect.fTop, _computedStyle.color().argb());
            const Color background = _computedStyle.selectionBackgroundColor();
            if (_anchor != _caret && background.alpha() > 0) {
                auto [begin, end] = selection();
                _textLayout.paintSelection(
                    canvas, left, _paddedRect.fTop, begin, end, background.argb(), _computedStyle.selectionColor().argb()
                );
            }
        }

        if (_caretVisible) {
            // A 2 px bar, in the text's color unless caretColor says otherwise
            const SkRect caret = _textLayout.caretRect(_caret).makeOffset(left, _paddedRect.fTop);
            SkPaint      paint{};
            paint.setAntiAlias(true);
            paint.setColor((_computedStyle.has(Property::caretColor) ? _computedStyle.caretColor() : _computedStyle.color()).argb());
            canvas->drawRect(caret.makeOutset(1.0f, 0.0f), paint);
        }
        if (scrolled || cut) {
            canvas->restore();
        }
    }
}
