#pragma once

#include "psychic-ui/skins/base/ButtonSkinBase.hpp"

namespace psychic_ui {
    /**
     * A tab (the "tab" variant, which `Tabs` use): its label over an underline in its `borderColor`, which isn't
     * drawn as a border, so the label doesn't move
     */
    class TabButtonSkin : public ButtonSkinBase {
    public:
        TabButtonSkin();
    protected:
        void drawChrome(SkCanvas *canvas, const paint::Box &box, SkinState state) override;
    };
}
