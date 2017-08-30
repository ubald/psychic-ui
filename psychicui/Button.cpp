#include <iostream>
#include "Button.hpp"

namespace psychicui {
    Button::Button(const std::string &label)
        : Component() {
        setTag("Button");
        //TODO: Implement a createChildren method that'll be universal, whatever the constructor used
        _label = std::make_shared<Label>(label);
        // TODO: _label->style()->setValue(position, "absolute");
        addChild(_label);
    }

    std::string Button::label() {
        return _label->text();
    }

    void Button::setLabel(std::string label) {
        _label->setText(label);
    }

    void Button::styleUpdated() {
        Component::styleUpdated();

        paint.setColor(_computedStyle->getValue(backgroundColor));
        paint.setStyle(SkPaint::kFill_Style);
        if (_computedStyle->getValue(cornerRadius) > 0) {
            paint.setAntiAlias(true);
        } else {
            paint.setAntiAlias(_computedStyle->getValue(antiAlias));
        }
    }

    void Button::draw(SkCanvas *canvas) {
        Component::draw(canvas);

        if (_computedStyle->getValue(cornerRadius) > 0) {
            canvas->drawRoundRect(_rect, _computedStyle->getValue(cornerRadius), _computedStyle->getValue(cornerRadius), paint);
        } else {
            canvas->drawRect(_rect, paint);
        }
    }
}