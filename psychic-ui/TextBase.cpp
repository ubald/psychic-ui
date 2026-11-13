#include "TextBase.hpp"


namespace psychic_ui {

    TextBase::TextBase() :
        Div::Div() {
        tag("TextBase");

        _defaultStyle.shrink(0)
            .grow(0);

        setMeasurable();

        #ifdef DEBUG_LAYOUT
        dashed = true;
        #endif
    }

    bool TextBase::lcdRender() const {
        return _lcdRender;
    }

    bool TextBase::subPixelText() const {
        return _subPixelText;
    }

    void TextBase::updateTextFlags(const bool lcdRender, const bool subPixelText) {
        if (_lcdRender != lcdRender || _subPixelText != subPixelText) {
            _lcdRender    = lcdRender;
            _subPixelText = subPixelText;
            invalidateStyle();
        }
    }

    unsigned int TextBase::getLineHeight() const {
        // TODO: Fix that type issue
        return static_cast<unsigned int>(std::ceil(_lineHeight));
    }

    float TextBase::linesHeight(size_t lines) const {
        return std::max(lines * _lineHeight, std::ceil(_font.getSpacing()));
    }

    void TextBase::styleUpdated() {
        Div::styleUpdated();

        bool antiAlias = _computedStyle.textAntiAlias();
        _fontSize   = _computedStyle.fontSize();
        // Auto is 1.5 times the font size, a percent is of the font size
        _lineHeight = _computedStyle.lineHeight().resolve(_fontSize, _fontSize * 1.5f);

        _textPaint.setAntiAlias(antiAlias);
        _textPaint.setColor(_computedStyle.color().argb());
        _font.setEdging(
            !antiAlias ? SkFont::Edging::kAlias
                       : _lcdRender ? SkFont::Edging::kSubpixelAntiAlias
                                    : SkFont::Edging::kAntiAlias
        );
        _font.setSubpixel(antiAlias && _subPixelText);
        _font.setTypeface(styleManager()->font(_computedStyle.fontFamily().str()));
        _font.setSize(_fontSize);

        // Yoga caches measures: re-measure when anything changing the text metrics changes
        FontKey fontKey{
            _font.getTypeface(),
            _fontSize,
            _computedStyle.letterSpacing(),
            _lineHeight,
            _font.isSubpixel()
        };
        if (fontKey != _fontKey) {
            _fontKey = fontKey;
            invalidate();
        }
    }


}
