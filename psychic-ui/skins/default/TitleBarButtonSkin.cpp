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
        const float   radius = std::min(_width, _height) / 2.0f;
        const SkPoint center = SkPoint::Make(_width / 2.0f, _height / 2.0f);
        paint::disc(canvas, center, radius, component()->computedStyle().backgroundColor());
    }

}
