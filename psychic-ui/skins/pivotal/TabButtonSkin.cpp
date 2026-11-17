#include "TabButtonSkin.hpp"

namespace psychic_ui {
    TabButtonSkin::TabButtonSkin() :
        ButtonSkinBase() {
        tag("TabButtonSkin");
    }

    void TabButtonSkin::drawChrome(SkCanvas *canvas, const paint::Box &box, const SkinState state) {
        ButtonSkinBase::drawChrome(canvas, box, state);
        // 2 px, 4 px in from the sides, rounded on top, on the tab's last pixels (over the strip's line)
        const Color color = _computedStyle.borderColor();
        if (color.alpha() == 0) {
            return;
        }
        const SkRect  line = SkRect::MakeLTRB(4.0f, _height - 2.0f, _width - 4.0f, _height);
        const SkVector top{1.0f, 1.0f};
        const SkVector bottom{0.0f, 0.0f};
        const SkVector radii[4]{top, top, bottom, bottom};
        SkRRect shape;
        shape.setRectRadii(line, radii);
        paint::fill(canvas, shape, color, _computedStyle.antiAlias());
    }
}
