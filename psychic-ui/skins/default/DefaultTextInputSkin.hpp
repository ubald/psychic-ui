#pragma once

#include "psychic-ui/skins/base/TextFieldSkinBase.hpp"

namespace psychic_ui {
    /**
     * The default text input: the default chrome around the text
     */
    class DefaultTextInputSkin : public TextInputSkinBase {
    public:
        DefaultTextInputSkin();
    protected:
        void drawChrome(SkCanvas *canvas, const paint::Box &box, SkinState state) override;
    };
}
