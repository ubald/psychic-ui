#include <iostream>
#include "Button.hpp"

namespace psychicui {
    Button::Button(Widget *parent, const std::string &label)
        : Widget(parent) {}

    void Button::draw(NVGcontext *ctx) {
        std::cout << _size.x() << " " << _size.y() << std::endl;
        nvgBeginPath(ctx);
        nvgRoundedRect(ctx, _position.x(), _position.y(), _size.x(), _size.y(), style()->buttonCornerRadius);
        nvgFillColor(ctx, style()->buttonColor);
        nvgFill(ctx);
        Widget::draw(ctx);
    }
}