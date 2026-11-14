#pragma once

#include "psychic-ui/skins/base/CheckBoxSkinBase.hpp"

namespace psychic_ui {
    /**
     * The default check box, Pivotal's: a small rounded box, its background and border from its style
     * (`CheckBox.checked .box` fills it with the accent), and a check mark in the box's `color` when checked
     */
    class DefaultCheckBoxSkin : public CheckBoxSkinBase {
    public:
        DefaultCheckBoxSkin();
    protected:
        void drawChrome(SkCanvas *canvas, const paint::Box &box, SkinState state) override;
    };
}
