#include <string>
#include <iostream>
#include "Label.hpp"

namespace psychicui {

    Label::Label(const std::string &text):
    Component::Component() {
        setTag("Label");
        setMeasurable();
        setText(text);
    }

    std::string Label::text() {
        return _text;
    }

    void Label::setText(const std::string &text) {
        _text = text;
        _textBox.setText(_text.c_str(), _text.length(), _textPaint);
        invalidate();
    }

    void Label::styleUpdated() {
        Component::styleUpdated();

        _textPaint.setAntiAlias(_computedStyle->get(textAntiAlias));
        if ( _computedStyle->get(textAntiAlias) ) {
            _textPaint.setLCDRenderText(true);
            _textPaint.setSubpixelText(true);
        }

        _textPaint.setTypeface(styleManager()->font(_computedStyle->get(fontFamily)));
        _textPaint.setTextSize(_computedStyle->get(fontSize));
        _textPaint.setColor(_computedStyle->get(color));

        // TextBox doesn't measure the same way it draws, we have to set the spacing manually
        _textBox.setSpacing(
            _computedStyle->get(fontSize) / _textPaint.getFontSpacing(),
            _computedStyle->get(lineHeight) - _computedStyle->get(lineHeight)
        );

        YGNodeStyleSetMinHeight(_yogaNode, std::max(_computedStyle->get(minHeight), _computedStyle->get(lineHeight)));
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
                // size.height = _textBox.getTextHeight();
                // TextBox doesn't measure the same way it draws, we have to set the spacing manually
                size.height = _textBox.countLines() * _computedStyle->get(lineHeight);
            } else {
                size.width = w;
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