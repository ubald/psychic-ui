#include <iostream>
#include "Button.hpp"

namespace psychicui {
    Button::Button(const std::string &label)
        : Component() {
        setComponentType("Button");
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
        Component::draw(canvas);

        // TODO: Cache setStyle
        Style *s = style().get();

        SkPaint paint;
        if (_mouseDown) {
            paint.setColor(s->getValue(buttonDownColor));
        } else if (_mouseOver) {
            paint.setColor(s->getValue(buttonOverColor));
        } else {
            paint.setColor(s->getValue(buttonColor));
        }
        paint.setStyle(SkPaint::kFill_Style);
        if (s->getValue(buttonCornerRadius) > 0) {
            paint.setAntiAlias(true);
            canvas->drawRoundRect(_rect, s->getValue(buttonCornerRadius), s->getValue(buttonCornerRadius), paint);
        } else {
            canvas->drawRect(_rect, paint);
        }
    }
}