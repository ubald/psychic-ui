#include "psychic-ui/components/Menu.hpp"
#include "MenuButtonSkinBase.hpp"

namespace psychic_ui {
    MenuButtonSkinBase::MenuButtonSkinBase() :
        MenuButtonSkin() {
        _icon = &add<Icon>();
        _icon->addClassName("icon")
             .display(Display::None);
        _label = &add<Label>();
        _label->addClassName("label");
        _shortcut = &add<Label>();
        _shortcut->addClassName("shortcut");
    }

    void MenuButtonSkinBase::added() {
        const MenuItem *item = component()->menuItem();
        _icon->name(item->icon)
             .display(item->icon.empty() ? Display::None : Display::Flex);
        _label->text(item->label);
        _shortcut->text(item->shortcut)
                 .display(item->shortcut.empty() ? Display::None : Display::Flex);
    }

    void MenuButtonSkinBase::draw(SkCanvas *canvas) {
        drawChrome(canvas, paint::Box::of(*this), SkinState::of(component()));
    }

    void MenuButtonSkinBase::drawChrome(SkCanvas *canvas, const paint::Box &/*box*/, SkinState /*state*/) {
        Div::draw(canvas);
    }
}
