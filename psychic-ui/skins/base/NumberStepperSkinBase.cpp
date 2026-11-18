#include "include/core/SkCanvas.h"
#include "psychic-ui/components/Icon.hpp"
#include "NumberStepperSkinBase.hpp"
#include "psychic-ui/Div.hpp"
#include "psychic-ui/components/NumberStepper.hpp"
#include "psychic-ui/components/Text.hpp"
#include "psychic-ui/paint/Box.hpp"
#include "SkinState.hpp"

namespace psychic_ui {
    NumberStepperSkinBase::NumberStepperSkinBase() :
        NumberStepperSkin() {
        tag("NumberStepperSkinBase");
        _decrement = &add<Div>();
        _decrement->addClassName("decrement");
        _decrement->add<Icon>("minus").mouseEnabled(false);

        _value = &add<Text>(); // Its stepper takes the mouse
        _value->addClassName("value");

        _increment = &add<Div>();
        _increment->addClassName("increment");
        _increment->add<Icon>("plus").mouseEnabled(false);
    }

    Text *NumberStepperSkinBase::textView() {
        return _value;
    }

    void NumberStepperSkinBase::setBounds(const bool canDecrement, const bool canIncrement) {
        _decrement->enabled(canDecrement);
        _increment->enabled(canIncrement);
    }

    NumberStepperSkin::Part NumberStepperSkinBase::partAt(const float x, const float y) const {
        // Our children: their places are in our coordinates
        if (_decrement->contains(x, y)) {
            return Part::Decrement;
        }
        return _increment->contains(x, y) ? Part::Increment : Part::None;
    }

    void NumberStepperSkinBase::draw(SkCanvas *canvas) {
        drawChrome(canvas, paint::Box::of(*this), SkinState::of(component()));
    }

    void NumberStepperSkinBase::drawChrome(SkCanvas *canvas, const paint::Box &/*box*/, SkinState /*state*/) {
        Div::draw(canvas);
    }
}
