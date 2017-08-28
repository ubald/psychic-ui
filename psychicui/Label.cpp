#include <string>
#include <iostream>
#include "Label.hpp"

namespace psychicui {

    Label::Label(const std::string &text):
    Widget::Widget() {
        setMeasurable();
        setText(text);


        // TODO: Implement invalidation method for when setStyle changes
        Style *s = style().get();
        std::cout << s->font->fontStyle().weight() << std::endl;
        _paint.setAntiAlias(s->textAntiAlias);
        if ( s->textAntiAlias ) {
            _paint.setLCDRenderText(true);
            _paint.setSubpixelText(true);
        }
        _paint.setTypeface(s->font);
        _paint.setTextSize(s->textSize);
        _paint.setEmbeddedBitmapText(true);
        _paint.setColor(s->textColor);

        // TextBox doesn't measure the same way it draws, we have to set the spacing manually
        _textBox.setSpacing(s->textSize / _paint.getFontSpacing(), s->textLeading - s->textSize);
        setMinHeight(s->textLeading);
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
        YGSize size = Widget::measure(width, widthMode, height, heightMode);

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
                size.height = _textBox.countLines() * s->textLeading;
            } else {
                size.width = w;
                size.height = _paint.getFontSpacing();
            }
        }


        return size;
    }

    void Label::draw(SkCanvas *canvas) {
        Widget::draw(canvas);
        _textBox.setBox(_paddingLeft, _paddingTop, _width - _paddingRight, _height - _paddingBottom);
        _textBox.draw(canvas);
    }
}