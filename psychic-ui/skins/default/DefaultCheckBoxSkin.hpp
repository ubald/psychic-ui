#pragma once

#include "psychic-ui/skins/base/CheckBoxSkinBase.hpp"

namespace psychic_ui {
    /**
     * The default check box: the default chrome on the box, filled with the box's background color when checked
     * or under the mouse
     */
    class DefaultCheckBoxSkin : public CheckBoxSkinBase {
    public:
        DefaultCheckBoxSkin();
    protected:
        void drawChrome(SkCanvas *canvas, const paint::Box &box, SkinState state) override;
    };
}
