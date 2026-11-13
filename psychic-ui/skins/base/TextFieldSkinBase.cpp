#include "TextFieldSkinBase.hpp"

namespace psychic_ui {

    void TextInputSkinBase::addedToComponent() {
        add(component()->textDisplay());
    }

    void TextInputSkinBase::removedFromComponent() {
        remove(component()->textDisplay());
    }

    void TextInputSkinBase::draw(SkCanvas *canvas) {
        drawChrome(canvas, paint::Box::of(*this), SkinState::of(component()));
    }

    void TextInputSkinBase::drawChrome(SkCanvas *canvas, const paint::Box &/*box*/, SkinState /*state*/) {
        Div::draw(canvas);
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
