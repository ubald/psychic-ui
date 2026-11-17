#pragma once

#include "psychic-ui/components/Button.hpp"
#include "psychic-ui/components/Icon.hpp"
#include "psychic-ui/components/Label.hpp"
#include "psychic-ui/paint/Box.hpp"
#include "SkinState.hpp"

namespace psychic_ui {
    /**
     * What every button skin shares: the `.icon` part, showing the button's icon (not displayed without one),
     * then the `.label` part, showing its label (not displayed when empty).
     * A skin set's button only paints its chrome, behind them.
     */
    class ButtonSkinBase : public ButtonSkin {
    public:
        ButtonSkinBase();

        void setLabel(const std::string &label) override;
        void setIcon(const std::string &icon) override;

    protected:
        Icon  *_icon{nullptr};  // The `.icon` part, a child, which we own
        Label *_label{nullptr}; // The `.label` part, a child, which we own

        void draw(SkCanvas *canvas) final;

        /**
         * Paint the button behind its label. The default is the Div's own painting: its background and border
         * from the computed style.
         */
        virtual void drawChrome(SkCanvas *canvas, const paint::Box &box, SkinState state);
    };
}
