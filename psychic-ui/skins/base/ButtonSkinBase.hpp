#pragma once

#include "psychic-ui/components/Button.hpp"
#include "psychic-ui/components/Label.hpp"
#include "psychic-ui/skins/paint/Box.hpp"
#include "SkinState.hpp"

namespace psychic_ui {
    /**
     * What every button skin shares: the `.label` part, which shows the button's label.
     * A skin set's button only paints its chrome, behind the label.
     */
    class ButtonSkinBase : public ButtonSkin {
    public:
        ButtonSkinBase();

        void setLabel(const std::string &label) override;

    protected:
        Label *_label{nullptr}; // The `.label` part, a child, which we own

        void draw(SkCanvas *canvas) final;

        /**
         * Paint the button behind its label. The default is the Div's own painting: its background and border
         * from the computed style.
         */
        virtual void drawChrome(SkCanvas *canvas, const paint::Box &box, SkinState state);
    };
}
