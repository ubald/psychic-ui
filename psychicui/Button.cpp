#include <iostream>
#include "Button.hpp"

namespace psychicui {
    Button::Button(const std::string &label)
        : Widget() {
        //TODO: Implement a createChildren method that'll be universal, whatever the constructor used
        _label = std::make_shared<Label>(label);
        addChild(_label);
    }

    std::string Button::label() {
        return _label->text();
    }

    void Button::setLabel(std::string label) {
        _label->setText(label);
    }

    void Button::draw(SkCanvas *canvas) {
        Widget::draw(canvas);

        // TODO: Cache setStyle
        Style *s = style().get();

        SkPaint paint;
        if (_mouseDown) {
            paint.setColor(s->buttonDownColor);
        } else if (_mouseOver) {
            paint.setColor(s->buttonOverColor);
        } else {
            paint.setColor(s->buttonColor);
        }
        paint.setStyle(SkPaint::kFill_Style);
        if (s->buttonCornerRadius > 0) {
            paint.setAntiAlias(true);
            canvas->drawRoundRect(_rect, s->buttonCornerRadius, s->buttonCornerRadius, paint);
        } else {
            canvas->drawRect(_rect, paint);
        }
    }
}