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
        _textBox.setText(_text.c_str(), _text.length(), _paint);
        invalidate();
    }

    void Label::styleUpdated() {
        Component::styleUpdated();

        _paint.setAntiAlias(_computedStyle->getValue(textAntiAlias));
        if ( _computedStyle->getValue(textAntiAlias) ) {
            _paint.setLCDRenderText(true);
            _paint.setSubpixelText(true);
        }

        _paint.setTypeface(styleManager() ? styleManager()->getFont(_computedStyle->getValue(fontFamily)) : nullptr);
        _paint.setTextSize(_computedStyle->getValue(fontSize));
        _paint.setColor(_computedStyle->getValue(color));

        // TextBox doesn't measure the same way it draws, we have to set the spacing manually
        _textBox.setSpacing(
            _computedStyle->getValue(fontSize) / _paint.getFontSpacing(),
            _computedStyle->getValue(lineHeight) - _computedStyle->getValue(lineHeight)
        );

        YGNodeStyleSetMinHeight(_yogaNode, std::max(_computedStyle->getValue(minHeight), _computedStyle->getValue(lineHeight)));
    }

    YGSize Label::measure(float width, YGMeasureMode widthMode, float height, YGMeasureMode heightMode) {
        YGSize size = Component::measure(width, widthMode, height, heightMode);

        if (widthMode == YGMeasureModeUndefined) {
            // Don't care about setWidth so do one line
            _textBox.setMode(SkTextBox::kOneLine_Mode);
            size.width = _paint.measureText(_text.c_str(), _text.length());
        } else {
            float w = _paint.measureText(_text.c_str(), _text.length());
            if (w > width) {
                Style *s = style();

                _textBox.setMode(SkTextBox::kLineBreak_Mode);
                _textBox.setBox(0, 0, width, height);
                // size.height = _textBox.getTextHeight();
                // TextBox doesn't measure the same way it draws, we have to set the spacing manually
                size.height = _textBox.countLines() * s->getValue(lineHeight);
            } else {
                size.width = w;
                size.height = _paint.getFontSpacing();
            }
        }


        return size;
    }

    void Label::draw(SkCanvas *canvas) {
        Component::draw(canvas);
        _textBox.setBox(
            _computedStyle->getValue(paddingLeft),
            _computedStyle->getValue(paddingTop),
            _width - _computedStyle->getValue(paddingRight),
            _height - _computedStyle->getValue(paddingBottom)
        );
        _textBox.draw(canvas);
    }
}