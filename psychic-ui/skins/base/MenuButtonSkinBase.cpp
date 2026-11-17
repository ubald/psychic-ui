#include "psychic-ui/components/Menu.hpp"
#include "MenuButtonSkinBase.hpp"

namespace psychic_ui {
    MenuButtonSkinBase::MenuButtonSkinBase() :
        MenuButtonSkin() {
        tag("MenuButtonSkinBase");
        _check = &add<Icon>();
        _check->addClassName("check")
              .display(Display::None);
        _icon = &add<Icon>();
        _icon->addClassName("icon")
             .display(Display::None);
        _label = &add<Label>();
        _label->addClassName("label");
        _shortcut = &add<Label>();
        _shortcut->addClassName("shortcut");
        _indicator = &add<Icon>();
        _indicator->addClassName("indicator")
                  .display(Display::None);
        _arrow = &add<Icon>("chevron-right");
        _arrow->addClassName("arrow")
              .display(Display::None);
    }

    void MenuButtonSkinBase::added() {
        itemChanged();
    }

    void MenuButtonSkinBase::itemChanged() {
        const MenuButton *button = component();
        if (!button) {
            return;
        }
        const MenuItem *item     = button->menuItem();
        const char     *mark     = button->checked() ? "check" : "";
        const bool     leading   = button->checkColumn() && !button->checkTrailing();
        const bool     trailing  = button->checkColumn() && button->checkTrailing();
        _check->name(leading ? mark : "")
              .display(leading ? Display::Flex : Display::None);
        _icon->name(item->icon)
             .display(item->icon.empty() && !button->iconColumn() ? Display::None : Display::Flex);
        _label->text(item->label);
        _shortcut->text(item->shortcut)
                 .display(item->shortcut.empty() ? Display::None : Display::Flex);
        _indicator->name(trailing ? mark : "")
                  .display(trailing ? Display::Flex : Display::None);
        _arrow->display(dynamic_cast<const SubMenuItem *>(item) ? Display::Flex : Display::None);
    }

    void MenuButtonSkinBase::draw(SkCanvas *canvas) {
        drawChrome(canvas, paint::Box::of(*this), SkinState::of(component()));
    }

    void MenuButtonSkinBase::drawChrome(SkCanvas *canvas, const paint::Box &/*box*/, SkinState /*state*/) {
        Div::draw(canvas);
    }
}
