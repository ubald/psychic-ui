#pragma once

#include <SkTypeface.h>
#include <psychicui/psychicui.hpp>

namespace psychicui {
    class Style {
    public:
        static std::shared_ptr<Style> defaultStyle;
        static sk_sp<SkTypeface>      defaultFont;

        Style();

        Color transparent{0x00000000};
        Color windowBackgroundColor{0xFF000000};
        Color panelDropShadowColor{0xFF000000};
        Color popupBackgroundColor{0xFF000000};


        sk_sp<SkTypeface> font{nullptr};
        bool              textAntiAlias{true};
        int               textSize{12};
        int               textLeading{16};
        Color             textColor{0xFFFFFFFF};

        Color buttonColor{0xFF313033};
        Color buttonOverColor{0xFF00FF00};
        Color buttonDownColor{0xFF0000FF};

        int titleBarHeight{24};
        int panelCornerRadius{0};
        int panelDropShadowSize{0};
        int buttonCornerRadius{10};
    protected:
    };
}
