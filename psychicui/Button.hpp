#pragma once

#include "psychicui.hpp"
#include "Widget.hpp"

namespace psychicui {
    class Button: public Widget {
    public:
        Button(const std::string &label = "Untitled");

        void draw(NVGcontext *ctx) override;
    };
}

