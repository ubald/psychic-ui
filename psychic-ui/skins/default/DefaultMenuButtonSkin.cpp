#include "DefaultMenuButtonSkin.hpp"

namespace psychic_ui {
    DefaultMenuButtonSkin::DefaultMenuButtonSkin() :
        MenuButtonSkinBase() {
        tag("DefaultMenuButtonSkin");
    }

    DefaultSubMenuButtonSkin::DefaultSubMenuButtonSkin() :
        DefaultMenuButtonSkin() {
        _arrow = &add<Icon>("chevron-right");
        _arrow->addClassName("arrow");
    }
}
