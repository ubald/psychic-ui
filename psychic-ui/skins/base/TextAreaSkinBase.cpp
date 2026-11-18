#include "TextAreaSkinBase.hpp"

namespace psychic_ui {

    TextAreaSkinBase::TextAreaSkinBase() :
        TextAreaSkin() {
        tag("TextAreaSkinBase");
    }

    void TextAreaSkinBase::addedToComponent() {
        add(component()->textScroller());
    }

    void TextAreaSkinBase::removedFromComponent() {
        remove(component()->textScroller());
    }

    void TextAreaSkinBase::draw(SkCanvas *canvas) {
        drawChrome(canvas, paint::Box::of(*this), SkinState::of(component()));
    }

    void TextAreaSkinBase::drawChrome(SkCanvas *canvas, const paint::Box &/*box*/, SkinState /*state*/) {
        Div::draw(canvas);
    }
}
