#pragma once

#include "psychic-ui/skins/base/ButtonSkinBase.hpp"

namespace psychic_ui {
    /**
     * A button without chrome: its label over the background and border its style gives it
     * (the "basic" variant, for tool bars, tabs and menu bars)
     */
    class DefaultBasicButtonSkin : public ButtonSkinBase {
    public:
        DefaultBasicButtonSkin();
    };
}
