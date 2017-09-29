#include "TextBase.hpp"


namespace psychic_ui {

    TextBase::TextBase(const std::string &text) :
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

    bool TextBase::subpixelText() const {
        return _subPixelText;
    }

    TextBase *TextBase::setSubPixelText(const bool subPixelText) {
        if (_subPixelText != subPixelText) {
            _subPixelText = subPixelText;
            invalidateStyle();
        }
        return this;
    }

    void TextBase::styleUpdated() {
        Div::styleUpdated();

        bool  antiAlias = _computedStyle->get(textAntiAlias);
        _fontSize      = _computedStyle->get(fontSize);
        _lineHeight   = _computedStyle->get(lineHeight);
        if (std::isnan(_lineHeight)) {
            _lineHeight = _fontSize * 1.5f;
        }

        _textPaint.setAntiAlias(antiAlias);
        _textPaint.setLCDRenderText(antiAlias && _lcdRender);
        _textPaint.setSubpixelText(antiAlias && _subPixelText);
        _textPaint.setTypeface(styleManager()->font(_computedStyle->get(fontFamily)));
        _textPaint.setTextSize(_fontSize);
        _textPaint.setColor(_computedStyle->get(color));

        float mh = _computedStyle->get(minHeight);
        _defaultStyle->set(minHeight, std::isnan(mh) ? _lineHeight : std::max(mh, _lineHeight));
    }


}
