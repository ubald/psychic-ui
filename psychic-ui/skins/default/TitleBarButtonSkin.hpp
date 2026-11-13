#pragma once

#include "psychic-ui/components/Button.hpp"

namespace psychic_ui {
    /**
     * A title bar's window button (the "title-bar" variant): a ringed dot of the button's background color.
     * It has no label, so it skips ButtonSkinBase.
     */
    class TitleBarButtonSkin : public ButtonSkin {
    public:
        TitleBarButtonSkin();
    protected:
        void draw(SkCanvas *canvas) override;
    };
}
