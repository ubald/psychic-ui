#include "psychic-ui/skins/paint/Marks.hpp"
#include "TitleBarButtonSkin.hpp"

namespace psychic_ui {
    TitleBarButtonSkin::TitleBarButtonSkin() :
        ButtonSkin() {
        tag("TitleBarButtonSkin");
        _inlineStyle.width(100_pct)
            .height(100_pct);
    }

    void TitleBarButtonSkin::draw(SkCanvas *canvas) {
        const int     hw     = _width / 2;
        const int     hh     = _height / 2;
        const SkPoint center = SkPoint::Make(static_cast<float>(hw), static_cast<float>(hh));

        paint::circle(canvas, center, static_cast<float>(hw), argb(0x10FFFFFF));
        paint::circle(canvas, center, static_cast<float>(hw - 1), argb(0xA0000000));
        paint::disc(canvas, center, static_cast<float>(hw - 2), component()->computedStyle().backgroundColor());
    }

}
