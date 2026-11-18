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

    unsigned int TextBase::getLineHeight() const {
        // TODO: Fix that type issue
        return static_cast<unsigned int>(std::ceil(_lineHeight));
    }

    float TextBase::linesHeight(size_t lines) const {
        return std::max(lines * _lineHeight, std::ceil(_font.getSpacing()));
    }

    void TextBase::styleUpdated() {
        Div::styleUpdated();

        _fontSize   = _computedStyle.fontSize();
        // Auto is 1.5 times the font size, a percent is of the font size
        _lineHeight = _computedStyle.lineHeight().resolve(_fontSize, _fontSize * 1.5f);

        _textPaint.setAntiAlias(true);
        _textPaint.setColor(_computedStyle.color().argb());
        _font.setTypeface(styleManager()->font(_computedStyle.fontFamily().str()));
        _font.setSize(_fontSize);

        TextLayoutStyle layoutStyle{
            styleManager()->fontCollection(),
            _computedStyle.fontFamily().str(),
            _fontSize,
            _lineHeight,
            _computedStyle.letterSpacing(),
            _computedStyle.fontSmoothing(),
            _computedStyle.textAlign()
        };
        // Yoga caches measures: re-measure when anything changing the text metrics changes
        bool remeasure = !(layoutStyle == _layoutStyle);
        _layoutStyle = layoutStyle;
        _textLayout.setStyle(layoutStyle, _computedStyle.color().argb(), _computedStyle.selectionColor().argb());
        if (remeasure) {
            invalidate();
        }
    }


}
