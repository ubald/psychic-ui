#pragma once

#include <cstdint>
#include <functional>
#include <string>
#include "psychic-ui/psychic-ui.hpp"
#include "psychic-ui/Component.hpp"
#include "psychic-ui/Skin.hpp"
#include "psychic-ui/signals/Signal.hpp"
#include "Text.hpp"
#include "TextField.hpp"

namespace psychic_ui {
    class NumberStepper;

    class NumberStepperSkin : public Skin<NumberStepper> {
    public:
        /**
         * What a press lands on
         */
        enum class Part : std::uint8_t { None, Decrement, Increment };

        NumberStepperSkin() : Skin<NumberStepper>() {
            tag("NumberStepperSkin");
        }

        /**
         * The `.value` part, which the skin creates and owns: the view showing the text, the selection and the caret
         */
        virtual Text *textView() = 0;

        /**
         * Whether the value can go down and up: the button parts are `:disabled` at the bounds
         */
        virtual void setBounds(bool /*canDecrement*/, bool /*canIncrement*/) {}

        /**
         * Which button a press at this point (in the skin's coordinates) is on, if any
         */
        virtual Part partAt(float x, float y) const = 0;
    };

    /**
     * @class NumberStepper
     *
     * A number with − and + on either side: the buttons, Up and Down (Shift for ten steps), and the mouse wheel
     * while it has the focus step it; typing a number and pressing Enter (or leaving the field) sets it, Escape
     * gives the typed text up. The value stays within `min()` and `max()`, and what is typed or stepped is rounded
     * to the decimals shown; a button at its bound is disabled. There is no press-and-hold repeat (no timer yet):
     * holding Up or Down repeats instead. Its skin shows the buttons (`.decrement`, `.increment`) and the text
     * (`.value`), and says which button a press is on.
     */
    class NumberStepper : public TextField<NumberStepperSkin> {
    public:
        explicit NumberStepper(double value = 0, std::function<void(double)> onChangeCallback = nullptr);

        double value() const;

        /**
         * Set the value, clamped to the range, emitting `changed` if it changed
         */
        template<class Self>
        Self &value(this Self &self, double value) {
            static_cast<NumberStepper &>(self).updateValue(value);
            return self;
        }

        /**
         * Set the value, clamped to the range, without emitting `changed`: for a value that mirrors one changed
         * elsewhere, so only the user's edits emit
         */
        template<class Self>
        Self &sync(this Self &self, double value) {
            static_cast<NumberStepper &>(self).updateValue(value, false);
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
         * Step the value up (or down with a negative count) by `count` steps, clamped and rounded to the decimals
         * shown; a step finer than them moves by one of their units
         */
        void stepBy(int count);

        /**
         * The value as the field shows it
         */
        std::string valueString() const;

        /**
         * With the focus, the wheel steps it; otherwise it scrolls what is around it
         */
        MouseEventStatus handleMouseScroll(const MouseEvent &event) override;

        /**
         * The value changed, with the new value (the text's own edits aren't changes until they are set): from the
         * user, or `value(v)`; `sync(v)` doesn't emit it
         */
        Signal<double> changed{};
        PSYCHIC_UI_EVENT(onChange, changed)

        /**
         * While what is typed isn't the value shown: Escape reverts it before closing a popup
         */
        bool takesEscape() const override;

    protected:
        double _value{0};
        double _min{-1e9};
        double _max{1e9};
        double _step{1};
        int    _decimals{-1};

        void updateValue(double value, bool emit = true);
        void updateRange(double min, double max);
        void updateStep(double step);
        void updateDecimals(int decimals);

        /**
         * 10 to the power of the decimals shown
         */
        double scale() const;

        /**
         * Set the value from what was typed, rounded to the decimals shown, or show the value again if it isn't a
         * number
         */
        void commit();

        /**
         * Show the value, and the bounds
         */
        void refresh();

        bool fieldKey(Key key, Mod mod) override;
        bool fieldPress(const MouseEvent &event) override;
        void skinChanged() override;
        void enabledChanged() override;
    };
}
