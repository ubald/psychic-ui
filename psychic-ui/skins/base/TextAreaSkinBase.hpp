#pragma once

#include "psychic-ui/components/TextArea.hpp"
#include "psychic-ui/paint/Box.hpp"
#include "SkinState.hpp"

namespace psychic_ui {
    /**
     * What every text area skin shares: it shows the component's scroller, around its text display, both owned by
     * the component. A skin set's text area only paints its chrome, behind them.
     */
    class TextAreaSkinBase : public TextAreaSkin {
    public:
        TextAreaSkinBase();

    protected:
        void addedToComponent() override;
        void removedFromComponent() override;
        void draw(SkCanvas *canvas) final;

        /**
         * Paint the field behind its text. The default is the Div's own painting.
         */
        virtual void drawChrome(SkCanvas *canvas, const paint::Box &box, SkinState state);
    };
}
