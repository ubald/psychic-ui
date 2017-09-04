#include <string>
#include <iostream>
#include "Label.hpp"

namespace psychicui {

    Label::Label(const std::string &text) :
        Component::Component() {
        setTag("Label");

        _defaults
            ->set(shrink, 0)
            ->set(grow, 0);

        setMeasurable();
        setText(text);
    }

    const std::string &Label::text() const {
        return _text;
    }

    void Label::setText(const std::string &text) {
        if (text == _text) {
            return;
        }
        _text = text;
        _textBox.setText(_text.c_str(), _text.length(), _textPaint);
        invalidate();
    }

    const bool Label::lcdRender() const {
        return _lcdRender;
    }

    Label *Label::setLcdRender(const bool lcdRender) {
        _lcdRender = lcdRender;
        invalidateStyle();
        return this;
    }

    const bool Label::subpixelText() const {
        return _subpixelText;
    }

    Label *Label::setSubpixelText(const bool subpixelText) {
        _subpixelText = subpixelText;
        invalidateStyle();
        return this;
    }

    void Label::styleUpdated() {
        Component::styleUpdated();

        _antiAlias = _computedStyle->get(textAntiAlias);
        float size = _computedStyle->get(fontSize);
        _lineHeight = _computedStyle->get(lineHeight);
        if (isnan(_lineHeight)) {
            _lineHeight = size * 1.5f;
        }

        _textPaint.setAntiAlias(_antiAlias);
        _textPaint.setLCDRenderText(_antiAlias && _lcdRender);
        _textPaint.setSubpixelText(_antiAlias && _subpixelText);

        _textPaint.setTypeface(styleManager()->font(_computedStyle->get(fontFamily)));
        _textPaint.setTextSize(size);
        _textPaint.setColor(_computedStyle->get(color));

        // TextBox doesn't measure the same way it draws, we have to set the spacing manually
        _textBox.setSpacing(size / _textPaint.getFontSpacing(), _lineHeight - size);

        YGNodeStyleSetMinHeight(_yogaNode, std::max(_computedStyle->get(minHeight), _lineHeight));
    }

    YGSize Label::measure(float width, YGMeasureMode widthMode, float height, YGMeasureMode heightMode) {
        YGSize size = Component::measure(width, widthMode, height, heightMode);

        if (widthMode == YGMeasureModeUndefined) {
            // Don't care about setWidth so do one line
            _textBox.setMode(SkTextBox::kOneLine_Mode);
            size.width = _textPaint.measureText(_text.c_str(), _text.length());
        } else {
            float w = _textPaint.measureText(_text.c_str(), _text.length());
            if (w > width) {
                _textBox.setMode(SkTextBox::kLineBreak_Mode);
                // The passed sizes consider padding, which is different than when we draw
                _textBox.setBox(0, 0, width, height);
                // size.getHeight = _textBox.getTextHeight();
                // TextBox doesn't measure the same way it draws, we have to set the spacing manually
                size.height = _textBox.countLines() * _lineHeight;
            } else {
                size.width  = w;
                size.height = _textPaint.getFontSpacing();
            }
        }

        return size;
    }

    void Label::draw(SkCanvas *canvas) {
        Component::draw(canvas);
        _textBox.setBox(_paddedRect);
        _textBox.draw(canvas);
    }
}