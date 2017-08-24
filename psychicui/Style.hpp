#pragma once

#include <psychicui/psychicui.hpp>

namespace psychicui {
    class Style {
    public:
        static Style defaultStyle;

        Color transparent{0.0f, 0.0f, 0.0f, 0.0f};
        Color windowBackgroundColor{0.0f, 0.0f, 0.0f, 1.0f};
        Color windowDropShadowColor{};
        Color popupBackgroundColor{};
        Color buttonColor{1.0f, 0.0f, 0.0f, 1.0f};

        int titleBarHeight{24};
        int windowCornerRadius{0};
        int windowDropShadowSize{0};
        int buttonCornerRadius{0};
    public:
        EIGEN_MAKE_ALIGNED_OPERATOR_NEW
    };
}
