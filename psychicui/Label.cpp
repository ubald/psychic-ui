#include <string>
#include <iostream>
#include "Label.hpp"

namespace psychicui {


    static std::string componentType = "label";

    Label::Label(const std::string &text):
    Component::Component() {
        setComponentType("Label");
        setMeasurable();
        setText(text);

        // TODO: Implement invalidation method for when setStyle changes
        auto s = style();
        _paint.setAntiAlias(s->getValue(textAntiAlias));
        if ( s->getValue(textAntiAlias) ) {
            _paint.setLCDRenderText(true);
            _paint.setSubpixelText(true);
        }
//        _paint.setTypeface(s->getValue(font)); TODO: Find/implement a font manager
        _paint.setTextSize(s->getValue(textSize));
        _paint.setEmbeddedBitmapText(true);
        _paint.setColor(s->getValue(color));

        // TextBox doesn't measure the same way it draws, we have to set the spacing manually
        _textBox.setSpacing(s->getValue(textSize) / _paint.getFontSpacing(), s->getValue(textLeading) - s->getValue(textSize));
        setMinHeight(s->getValue(textLeading));
    }

    std::string Label::text() {
        return _text;
    }

    void Label::setText(const std::string &text) {
        _text = text;
        _textBox.setText(_text.c_str(), _text.length(), _paint);
        invalidate();
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
                Style *s = style().get();

                _textBox.setMode(SkTextBox::kLineBreak_Mode);
                _textBox.setBox(0, 0, width, height);
                // size.height = _textBox.getTextHeight();
                // TextBox doesn't measure the same way it draws, we have to set the spacing manually
                size.height = _textBox.countLines() * s->getValue(textLeading);
            } else {
                size.width = w;
                size.height = _paint.getFontSpacing();
            }
        }


        return size;
    }

    void Label::draw(SkCanvas *canvas) {
        Component::draw(canvas);
        _textBox.setBox(_paddingLeft, _paddingTop, _width - _paddingRight, _height - _paddingBottom);
        _textBox.draw(canvas);
    }
}