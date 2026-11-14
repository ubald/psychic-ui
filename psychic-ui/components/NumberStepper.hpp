#pragma once

#include <functional>
#include <memory>
#include <string>
#include "psychic-ui/psychic-ui.hpp"
#include "psychic-ui/Component.hpp"
#include "psychic-ui/Skin.hpp"
#include "psychic-ui/signals/Signal.hpp"
#include "TextInput.hpp"

namespace psychic_ui {
    class NumberStepper;

    class NumberStepperSkin : public Skin<NumberStepper> {
    public:
        NumberStepperSkin() : Skin<NumberStepper>() {
            tag("NumberStepperSkin");
        }
    };

    /**
     * @class NumberStepper
     *
     * A number with − and + on either side: the buttons, Up and Down (Shift for ten steps), and the mouse wheel
     * while it has the focus step it; typing a number and pressing Enter (or leaving the field) sets it, Escape
     * gives the typed text up. The value stays within `min()` and `max()`; a button at its bound is disabled.
     * There is no press-and-hold repeat (no timer yet): holding Up or Down repeats instead.
     * The stepper owns its parts (`.decrement`, `.value`, a TextInput, and `.increment`), which its skin shows.
     */
    class NumberStepper : public Component<NumberStepperSkin> {
    public:
        explicit NumberStepper(double value = 0, std::function<void(double)> onChangeCallback = nullptr);

        double value() const;

        /**
         * Set the value, clamped to the range
         */
        template<class Self>
        Self &value(this Self &self, double value) {
            static_cast<NumberStepper &>(self).updateValue(value);
            return self;
        }

        double min() const;

        template<class Self>
        Self &min(this Self &self, double min) {
            auto &stepper = static_cast<NumberStepper &>(self);
            stepper.updateRange(min, std::max(min, stepper._max));
            return self;
        }

        double max() const;

        template<class Self>
        Self &max(this Self &self, double max) {
            auto &stepper = static_cast<NumberStepper &>(self);
            stepper.updateRange(std::min(max, stepper._min), max);
            return self;
        }

        /**
         * Set the minimum and maximum (swapped if reversed), clamping the value
         */
        template<class Self>
        Self &range(this Self &self, double min, double max) {
            static_cast<NumberStepper &>(self).updateRange(min, max);
            return self;
        }

        double step() const;

        /**
         * How much a button, key or wheel notch changes the value (1 by default)
         */
        template<class Self>
        Self &step(this Self &self, double step) {
            static_cast<NumberStepper &>(self).updateStep(step);
            return self;
        }

        /**
         * How many decimals the value shows; -1 (the default) takes as many as the step has
         */
        int decimals() const;

        template<class Self>
        Self &decimals(this Self &self, int decimals) {
            static_cast<NumberStepper &>(self).updateDecimals(decimals);
            return self;
        }

        /**
         * Step the value up (or down with a negative count) by `count` steps, clamped
         */
        void stepBy(int count);

        /**
         * The value as the field shows it
         */
        std::string valueString() const;

        /**
         * The `.value` part, the text input
         */
        std::shared_ptr<TextInput> input();

        /**
         * The `.decrement` and `.increment` parts, the buttons
         */
        std::shared_ptr<Div> decrementButton();
        std::shared_ptr<Div> incrementButton();

        /**
         * The input's text: Tab, or a press anywhere on the stepper, lets one type
         */
        Div *focusDelegate() override;

        /**
         * With the focus, the wheel steps it; otherwise it scrolls what is around it
         */
        MouseEventStatus handleMouseScroll(int mouseX, int mouseY, double scrollX, double scrollY) override;

        /**
         * The value changed, with the new value
         */
        Signal<double> changed{};
        PSYCHIC_UI_EVENT(onChange, changed)

    protected:
        double                     _value{0};
        double                     _min{-1e9};
        double                     _max{1e9};
        double                     _step{1};
        int                        _decimals{-1};
        std::shared_ptr<Div>       _decrement{nullptr};
        std::shared_ptr<TextInput> _input{nullptr};
        std::shared_ptr<Div>       _increment{nullptr};

        void updateValue(double value);
        void updateRange(double min, double max);
        void updateStep(double step);
        void updateDecimals(int decimals);

        /**
         * Set the value from what was typed, or show the value again if it isn't a number
         */
        void commit();

        /**
         * Show the value, and disable the buttons at the bounds
         */
        void refresh();

        void enabledChanged() override;
    };
}
