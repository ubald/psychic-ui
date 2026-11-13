#include "ButtonSkinBase.hpp"

namespace psychic_ui {
    ButtonSkinBase::ButtonSkinBase() :
        ButtonSkin() {
        _label = &add<Label>();
        _label->addClassName("label");
    }

    void ButtonSkinBase::setLabel(const std::string &label) {
        _label->text(label);
    }

    void ButtonSkinBase::draw(SkCanvas *canvas) {
        drawChrome(canvas, paint::Box::of(*this), SkinState::of(component()));
    }

    void ButtonSkinBase::drawChrome(SkCanvas *canvas, const paint::Box &/*box*/, SkinState /*state*/) {
        Div::draw(canvas);
    }
}
