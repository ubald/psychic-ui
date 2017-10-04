#include "TextBase.hpp"


namespace psychic_ui {

    TextBase::TextBase() :
        Div::Div() {
        setTag("TextBase");

        _defaultStyle
            ->set(shrink, 0)
            ->set(grow, 0);

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
        return static_cast<unsigned int>(std::ceil(_lineHeight));
    }

    void TextBase::styleUpdated() {
        Div::styleUpdated();

        bool antiAlias = _computedStyle->get(textAntiAlias);
        _fontSize   = _computedStyle->get(fontSize);
        _lineHeight = _computedStyle->get(lineHeight);
        if (std::isnan(_lineHeight)) {
            _lineHeight = _fontSize * 1.5f;
        }

        _textPaint.setAntiAlias(antiAlias);
        _textPaint.setLCDRenderText(antiAlias && _lcdRender);
        _textPaint.setSubpixelText(antiAlias && _subPixelText);
        _textPaint.setTypeface(styleManager()->font(_computedStyle->get(fontFamily)));
        _textPaint.setTextSize(_fontSize);
        _textPaint.setColor(_computedStyle->get(color));

        // If we don't have a percentage based min height use the line height
        if (!_computedStyle->has(minHeightPercent)) {
            float mh = _computedStyle->get(minHeight);
            _defaultStyle->set(minHeight, std::isnan(mh) ? _lineHeight : std::max(mh, std::max(_lineHeight, std::ceil(_textPaint.getFontSpacing()))));
        }
    }


}
