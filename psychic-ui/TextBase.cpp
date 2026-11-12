#include "TextBase.hpp"


namespace psychic_ui {

    TextBase::TextBase() :
        Div::Div() {
        setTag("TextBase");

        _defaultStyle
            .shrink(0)
            .grow(0);

        setMeasurable();

        #ifdef DEBUG_LAYOUT
        dashed = true;
        #endif
    }

    bool TextBase::lcdRender() const {
        return _lcdRender;
    }

    TextBase *TextBase::setLcdRender(const bool lcdRender) {
        if (_lcdRender != lcdRender) {
            _lcdRender = lcdRender;
            invalidateStyle();
        }
        return this;
    }

    bool TextBase::subPixelText() const {
        return _subPixelText;
    }

    TextBase *TextBase::setSubPixelText(const bool subPixelText) {
        if (_subPixelText != subPixelText) {
            _subPixelText = subPixelText;
            invalidateStyle();
        }
        return this;
    }

    unsigned int TextBase::getLineHeight() const {
        // TODO: Fix that type issue
        return static_cast<unsigned int>(std::ceil(_lineHeight));
    }

    float TextBase::fit(float content, float available, YGMeasureMode mode) {
        switch (mode) {
            case YGMeasureModeExactly:
                return available;
            case YGMeasureModeAtMost:
                return std::min(content, available);
            default:
                return content;
        }
    }

    float TextBase::linesHeight(size_t lines) const {
        return std::max(lines * _lineHeight, std::ceil(_textPaint.getFontSpacing()));
    }

    void TextBase::styleUpdated() {
        Div::styleUpdated();

        bool antiAlias = _computedStyle.textAntiAlias();
        _fontSize   = _computedStyle.fontSize();
        // Auto is 1.5 times the font size, a percent is of the font size
        _lineHeight = _computedStyle.lineHeight().resolve(_fontSize, _fontSize * 1.5f);

        _textPaint.setAntiAlias(antiAlias);
        _textPaint.setLCDRenderText(antiAlias && _lcdRender);
        _textPaint.setSubpixelText(antiAlias && _subPixelText);
        _textPaint.setTypeface(styleManager()->font(_computedStyle.fontFamily().str()));
        _textPaint.setTextSize(_fontSize);
        _textPaint.setColor(_computedStyle.color().argb());

        // Yoga caches measures: re-measure when anything changing the text metrics changes
        FontKey fontKey{
            _textPaint.getTypeface(),
            _fontSize,
            _computedStyle.letterSpacing(),
            _lineHeight,
            _textPaint.isSubpixelText()
        };
        if (fontKey != _fontKey) {
            _fontKey = fontKey;
            invalidate();
        }
    }


}
