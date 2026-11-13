#pragma once

#include "psychic-ui/Shape.hpp"
#include "psychic-ui/skins/base/MenuButtonSkinBase.hpp"

namespace psychic_ui {
    /**
     * A menu item: its label and shortcut over the background its style gives it
     */
    class DefaultMenuButtonSkin : public MenuButtonSkinBase {
    public:
        DefaultMenuButtonSkin();
    };

    /**
     * A menu item opening a submenu (the "submenu" variant): an `.arrow` part after the label
     */
    class DefaultSubMenuButtonSkin : public DefaultMenuButtonSkin {
    public:
        DefaultSubMenuButtonSkin();
    protected:
        Shape *_arrow{nullptr}; // The `.arrow` part, a child, which we own
    };
}
