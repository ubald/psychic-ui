#include "DefaultTextAreaSkin.hpp"
#include "DefaultChrome.hpp"

namespace psychic_ui {
    DefaultTextAreaSkin::DefaultTextAreaSkin() :
        TextAreaSkinBase() {
        tag("DefaultTextAreaSkin");
    }

    void DefaultTextAreaSkin::drawChrome(SkCanvas *canvas, const paint::Box &box, SkinState /*state*/) {
        default_skins::drawChrome(canvas, box, _computedStyle);
    }
}
