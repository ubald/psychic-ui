#include "ButtonSkinBase.hpp"

namespace psychic_ui {
    ButtonSkinBase::ButtonSkinBase() :
        ButtonSkin() {
        tag("ButtonSkinBase");
        _icon = &add<Icon>();
        _icon->addClassName("icon")
             .display(Display::None);
        _label = &add<Label>();
        _label->addClassName("label")
              .display(Display::None);
    }

    void ButtonSkinBase::setLabel(const std::string &label) {
        _label->text(label)
              .display(label.empty() ? Display::None : Display::Flex);
    }

    void ButtonSkinBase::setIcon(const std::string &icon) {
        _icon->name(icon)
             .display(icon.empty() ? Display::None : Display::Flex);
    }

    void ButtonSkinBase::draw(SkCanvas *canvas) {
        drawChrome(canvas, paint::Box::of(*this), SkinState::of(component()));
    }

    void ButtonSkinBase::drawChrome(SkCanvas *canvas, const paint::Box &/*box*/, SkinState /*state*/) {
        Div::draw(canvas);
    }
}
