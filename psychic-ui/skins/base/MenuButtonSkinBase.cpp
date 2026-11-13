#include "psychic-ui/components/Menu.hpp"
#include "MenuButtonSkinBase.hpp"

namespace psychic_ui {
    MenuButtonSkinBase::MenuButtonSkinBase() :
        MenuButtonSkin() {
        _label = &add<Label>();
        _label->addClassName("label");
        _shortcut = &add<Label>();
        _shortcut->addClassName("shortcut");
    }

    void MenuButtonSkinBase::added() {
        _label->text(component()->menuItem()->label);
        _shortcut->text(component()->menuItem()->shortcut);
    }

    void MenuButtonSkinBase::draw(SkCanvas *canvas) {
        drawChrome(canvas, paint::Box::of(*this), SkinState::of(component()));
    }

    void MenuButtonSkinBase::drawChrome(SkCanvas *canvas, const paint::Box &/*box*/, SkinState /*state*/) {
        Div::draw(canvas);
    }
}
