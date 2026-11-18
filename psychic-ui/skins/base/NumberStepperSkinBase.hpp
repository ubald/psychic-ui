#pragma once

#include "psychic-ui/components/NumberStepper.hpp"
#include "psychic-ui/components/Text.hpp"
#include "psychic-ui/paint/Box.hpp"
#include "SkinState.hpp"

namespace psychic_ui {
    /**
     * What every number stepper skin shares, its parts, in a row, all children, which its child list owns: the
     * `.decrement` button (a `minus` Icon), the `.value` part (the view its stepper drives) and the `.increment`
     * button (a `plus` Icon). The buttons are Divs the mouse reaches, for their hover and pressed looks, and
     * `:disabled` at the bounds; the stepper steps on a press there (`partAt`). A skin set's stepper paints the
     * field's chrome around them; its rules give the buttons their looks (`NumberStepper .decrement:hover`,
     * `:disabled`).
     */
    class NumberStepperSkinBase : public NumberStepperSkin {
    public:
        NumberStepperSkinBase();

        Text *textView() override;
        void setBounds(bool canDecrement, bool canIncrement) override;
        Part partAt(float x, float y) const override;

    protected:
        Div  *_decrement{nullptr}; // The `.decrement` part
        Text *_value{nullptr};     // The `.value` part
        Div  *_increment{nullptr}; // The `.increment` part

        void draw(SkCanvas *canvas) final;

        /**
         * Paint the field behind its parts. The default is the Div's own painting.
         */
        virtual void drawChrome(SkCanvas *canvas, const paint::Box &box, SkinState state);
    };
}
