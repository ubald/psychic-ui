#pragma once

#include "psychicui.hpp"
#include "Widget.hpp"

namespace psychicui {
    class Button: public Widget {
    public:
        Button(Widget *parent, const std::string &label = "Untitled");

        void draw(NVGcontext *ctx) override;
    };
}

