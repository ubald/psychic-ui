#pragma once

#include "psychic-ui/skins/base/ButtonSkinBase.hpp"

namespace psychic_ui {
    /**
     * The default button: the default chrome, filled with the background color inside it
     */
    class DefaultButtonSkin : public ButtonSkinBase {
    public:
        DefaultButtonSkin();
    protected:
        void drawChrome(SkCanvas *canvas, const paint::Box &box, SkinState state) override;
    };
}
