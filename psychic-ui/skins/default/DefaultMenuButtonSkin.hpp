#pragma once

#include "psychic-ui/skins/base/MenuButtonSkinBase.hpp"

namespace psychic_ui {
    /**
     * A menu item, Pivotal's: its icon, label and shortcut over the background its style gives it (raised while
     * highlighted)
     */
    class DefaultMenuButtonSkin : public MenuButtonSkinBase {
    public:
        DefaultMenuButtonSkin();
    };

    /**
     * A menu item opening a submenu (the "submenu" variant): an `.arrow` part after the label, a chevron icon
     */
    class DefaultSubMenuButtonSkin : public DefaultMenuButtonSkin {
    public:
        DefaultSubMenuButtonSkin();
    protected:
        Icon *_arrow{nullptr}; // The `.arrow` part, a child, which we own
    };
}
