#include "NumberStepperSkinBase.hpp"

namespace psychic_ui {
    NumberStepperSkinBase::NumberStepperSkinBase() :
        NumberStepperSkin() {
        tag("NumberStepperSkinBase");
    }

    void NumberStepperSkinBase::addedToComponent() {
        add(component()->decrementButton());
        add(component()->input());
        add(component()->incrementButton());
    }

    void NumberStepperSkinBase::removedFromComponent() {
        remove(component()->decrementButton());
        remove(component()->input());
        remove(component()->incrementButton());
    }

    void NumberStepperSkinBase::draw(SkCanvas *canvas) {
        drawChrome(canvas, paint::Box::of(*this), SkinState::of(component()));
    }

    void NumberStepperSkinBase::drawChrome(SkCanvas *canvas, const paint::Box &/*box*/, SkinState /*state*/) {
        Div::draw(canvas);
    }
}
