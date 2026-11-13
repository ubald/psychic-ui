#include "DefaultButtonSkin.hpp"
#include "DefaultChrome.hpp"

namespace psychic_ui {
    DefaultButtonSkin::DefaultButtonSkin() :
        ButtonSkinBase() {
        tag("DefaultButtonSkin");
    }

    void DefaultButtonSkin::drawChrome(SkCanvas *canvas, const paint::Box &box, SkinState /*state*/) {
        const SkRRect inner = default_skins::drawChrome(canvas, box, _computedStyle);
        paint::fill(canvas, inner, _computedStyle.backgroundColor(), _computedStyle.antiAlias());
    }
}
