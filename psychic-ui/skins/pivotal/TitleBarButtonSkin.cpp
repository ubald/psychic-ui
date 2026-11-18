#include "psychic-ui/paint/Marks.hpp"
#include "TitleBarButtonSkin.hpp"

namespace psychic_ui {
    TitleBarButtonSkin::TitleBarButtonSkin() :
        ButtonSkin() {
        tag("TitleBarButtonSkin");
        _inlineStyle.width(100_pct)
            .height(100_pct);
    }

    void TitleBarButtonSkin::draw(SkCanvas *canvas) {
        const SkRect  bounds = SkRect::Make(SkIRect::MakeWH(_width, _height));
        const float   radius = std::min(bounds.width(), bounds.height()) / 2.0f;
        const SkPoint center = bounds.center();
        paint::disc(canvas, center, radius, component()->computedStyle().backgroundColor());
    }

}
