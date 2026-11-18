#include "psychic-ui/components/Icon.hpp"
#include "NumberStepperSkinBase.hpp"

namespace psychic_ui {
    namespace {
        /**
         * Whether a point in a div's parent's coordinates is on it
         */
        bool isOn(const Div &div, const float x, const float y) {
            const auto left = static_cast<float>(div.x());
            const auto top  = static_cast<float>(div.y());
            return x >= left && x < left + static_cast<float>(div.getWidth())
                   && y >= top && y < top + static_cast<float>(div.getHeight());
        }
    }

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
        if (isOn(*_decrement, x, y)) {
            return Part::Decrement;
        }
        if (isOn(*_increment, x, y)) {
            return Part::Increment;
        }
        return Part::None;
    }

    void NumberStepperSkinBase::draw(SkCanvas *canvas) {
        drawChrome(canvas, paint::Box::of(*this), SkinState::of(component()));
    }

    void NumberStepperSkinBase::drawChrome(SkCanvas *canvas, const paint::Box &/*box*/, SkinState /*state*/) {
        Div::draw(canvas);
    }
}
