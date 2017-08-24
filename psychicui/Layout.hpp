#pragma once

#include "psychicui.hpp"

namespace psychicui {
    class Widget;

    class Layout {
    public:
        virtual void performLayout(NVGcontext *ctx, Widget *widget) const = 0;
        virtual Vector2i preferredSize(NVGcontext *ctx, const Widget *widget) const = 0;
    protected:
        virtual ~Layout() {}

    public:
        EIGEN_MAKE_ALIGNED_OPERATOR_NEW
    };
}
