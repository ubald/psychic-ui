#pragma once

#include <psychicui/psychicui.hpp>

namespace psychicui {
    class Style {
    public:
        static std::shared_ptr<Style> defaultStyle;

        Color transparent{0.0f, 0.0f, 0.0f, 0.0f};
        Color windowBackgroundColor{0.0f, 0.0f, 0.0f, 1.0f};
        Color panelDropShadowColor{};
        Color popupBackgroundColor{};
        Color buttonColor{1.0f, 0.0f, 0.0f, 1.0f};

        int titleBarHeight{24};
        int panelCornerRadius{0};
        int panelDropShadowSize{0};
        int buttonCornerRadius{0};
    public:
        EIGEN_MAKE_ALIGNED_OPERATOR_NEW
    };
}
