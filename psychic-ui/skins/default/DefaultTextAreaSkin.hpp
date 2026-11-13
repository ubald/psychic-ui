#pragma once

#include "psychic-ui/skins/base/TextFieldSkinBase.hpp"

namespace psychic_ui {
    /**
     * The default text area: the default chrome around the scroller
     */
    class DefaultTextAreaSkin : public TextAreaSkinBase {
    public:
        DefaultTextAreaSkin();
    protected:
        void drawChrome(SkCanvas *canvas, const paint::Box &box, SkinState state) override;
    };
}
