#include "DefaultCheckBoxSkin.hpp"
#include "DefaultChrome.hpp"

namespace psychic_ui {
    DefaultCheckBoxSkin::DefaultCheckBoxSkin() :
        CheckBoxSkinBase() {
        tag("DefaultCheckBoxSkin");
    }

    void DefaultCheckBoxSkin::drawChrome(SkCanvas *canvas, const paint::Box &box, const SkinState state) {
        const Style   &style = _box->computedStyle();
        const SkRRect inner  = default_skins::drawChrome(canvas, box, style);
        if (state.hover || component()->checked()) {
            paint::fill(canvas, inner, style.backgroundColor(), style.antiAlias());
        }
    }
}
