#pragma once

#include "psychic-ui/components/NumberStepper.hpp"
#include "psychic-ui/paint/Box.hpp"
#include "SkinState.hpp"

namespace psychic_ui {
    /**
     * What every number stepper skin shares: it shows the stepper's parts in a row, the `.decrement` button, the
     * `.value` input and the `.increment` button (the stepper owns them, so that they keep their state when the
     * skin changes). A skin set's stepper paints the field's chrome around them; its rules make the input borderless
     * (`NumberStepper .value`) and the buttons' looks (`NumberStepper .decrement:hover`, `:disabled`).
     */
    class NumberStepperSkinBase : public NumberStepperSkin {
    public:
        NumberStepperSkinBase();

    protected:
        void addedToComponent() override;
        void removedFromComponent() override;
        void draw(SkCanvas *canvas) final;

        /**
         * Paint the field behind its parts. The default is the Div's own painting.
         */
        virtual void drawChrome(SkCanvas *canvas, const paint::Box &box, SkinState state);
    };
}
