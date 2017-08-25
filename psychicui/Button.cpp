#include <iostream>
#include "Button.hpp"

namespace psychicui {
    Button::Button(const std::string &label)
        : Widget() {}

    void Button::draw(NVGcontext *ctx) {
        nvgBeginPath(ctx);
        nvgRoundedRect(ctx, _position.x(), _position.y(), _size.x(), _size.y(), style()->buttonCornerRadius);
        nvgFillColor(ctx, style()->buttonColor);
        nvgFill(ctx);
        Widget::draw(ctx);
    }
}