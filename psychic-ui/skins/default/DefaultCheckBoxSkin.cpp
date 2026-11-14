#include "psychic-ui/skins/paint/Icon.hpp"
#include "DefaultCheckBoxSkin.hpp"

namespace psychic_ui {
    DefaultCheckBoxSkin::DefaultCheckBoxSkin() :
        CheckBoxSkinBase() {
        tag("DefaultCheckBoxSkin");
    }

    void DefaultCheckBoxSkin::drawChrome(SkCanvas *canvas, const paint::Box &box, const SkinState state) {
        CheckBoxSkinBase::drawChrome(canvas, box, state);
        if (component()->checked()) {
            const Style &style = _box->computedStyle();
            // The mark is 2 px smaller than the box, as Pivotal draws it (12 px in 14), in the box's icon stroke
            const SkRect mark = box.outer.rect().makeInset(1.0f, 1.0f);
            paint::icon(canvas, "check", mark, style.color(), style.iconStrokeWidth());
        }
    }
}
