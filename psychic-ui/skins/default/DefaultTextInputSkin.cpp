#include "DefaultTextInputSkin.hpp"
#include "DefaultChrome.hpp"

namespace psychic_ui {
    DefaultTextInputSkin::DefaultTextInputSkin() :
        TextInputSkinBase() {
        tag("DefaultTextInputSkin");
    }

    void DefaultTextInputSkin::drawChrome(SkCanvas *canvas, const paint::Box &box, SkinState /*state*/) {
        default_skins::drawChrome(canvas, box, _computedStyle);
    }
}
