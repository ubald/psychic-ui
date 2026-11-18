#include <algorithm>
#include <charconv>
#include <cmath>
#include <format>
#include "NumberStepper.hpp"
#include "Icon.hpp"

namespace psychic_ui {

    namespace {
        /**
         * The decimals a step needs to show exactly, up to 6
         */
        int decimalsOf(const double step) {
            int    decimals = 0;
            double scaled   = std::abs(step);
            while (decimals < 6 && std::abs(scaled - std::round(scaled)) > 1e-9 * std::max(1.0, scaled)) {
                scaled *= 10;
                ++decimals;
            }
            return decimals;
        }
    }

    NumberStepper::NumberStepper(const double value, std::function<void(double)> onChangeCallback) :
        Component(),
        _value(value),
        _decrement(std::make_shared<Div>()),
        _input(std::make_shared<TextInput>()),
        _increment(std::make_shared<Div>()) {
        tag("NumberStepper");

        _focusEnabled = true; // The stepper takes the focus and hands it to its text

        if (onChangeCallback) {
            changed.subscribe(std::move(onChangeCallback));
        }

        _decrement->addClassName("decrement")
                  .cursor(Cursor::Arrow);
        _decrement->add<Icon>("minus").mouseEnabled(false);
        _decrement->onClick([this]() { stepBy(-1); });

        _increment->addClassName("increment")
                  .cursor(Cursor::Arrow);
        _increment->add<Icon>("plus").mouseEnabled(false);
        _increment->onClick([this]() { stepBy(1); });

        _input->addClassName("value")
              .focusEnabled(false); // Tab stops once, on the stepper

        // The text display moves its caret on Up and Down too (to the start or the end of its line); `refresh()`
        // sets the text again, which puts the caret back
        const auto key = [this](const Key key, const Mod modifiers) {
            if (!_enabled) {
                return;
            }
            const int steps = modifiers.shift ? 10 : 1;
            switch (key) {
                case Key::UP:
                    commit();
                    stepBy(steps);
                    break;
                case Key::DOWN:
                    commit();
                    stepBy(-steps);
                    break;
                case Key::ENTER:
                    commit();
                    break;
                case Key::ESCAPE:
                    refresh();
                    break;
                default:
                    break;
            }
        };
        _input->textDisplay()->onKeyPress(key);
        _input->textDisplay()->onKeyRepeat(key);
        _input->textDisplay()->onBlur([this]() { commit(); });

        refresh();
    }

    double NumberStepper::value() const {
        return _value;
    }

    void NumberStepper::updateValue(const double value) {
        const double v = std::clamp(value, _min, _max);
        if (v != _value) {
            _value = v;
            refresh();
            changed.emit(_value);
        } else {
            refresh(); // What was typed may differ from the value it gave
        }
    }

    double NumberStepper::min() const {
        return _min;
    }

    double NumberStepper::max() const {
        return _max;
    }

    void NumberStepper::updateRange(const double min, const double max) {
        _min = std::min(min, max);
        _max = std::max(min, max);
        updateValue(_value);
    }

    double NumberStepper::step() const {
        return _step;
    }

    void NumberStepper::updateStep(const double step) {
        _step = std::abs(step) > 0 ? std::abs(step) : 1;
        refresh();
    }

    int NumberStepper::decimals() const {
        return _decimals;
    }

    void NumberStepper::updateDecimals(const int decimals) {
        _decimals = decimals;
        refresh();
    }

    void NumberStepper::stepBy(const int count) {
        if (!_enabled) {
            return;
        }
        // Rounded to the decimals shown, so that steps of 0.1 don't drift
        const double scale = std::pow(10.0, _decimals >= 0 ? _decimals : decimalsOf(_step));
        updateValue(std::round((_value + count * _step) * scale) / scale);
    }

    std::string NumberStepper::valueString() const {
        return std::format("{:.{}f}", _value, _decimals >= 0 ? _decimals : decimalsOf(_step));
    }

    std::shared_ptr<TextInput> NumberStepper::input() {
        return _input;
    }

    std::shared_ptr<Div> NumberStepper::decrementButton() {
        return _decrement;
    }

    std::shared_ptr<Div> NumberStepper::incrementButton() {
        return _increment;
    }

    Div *NumberStepper::focusDelegate() {
        return _input->textDisplay().get();
    }

    MouseEventStatus NumberStepper::handleMouseScroll(const MouseEvent &event) {
        if (_enabled && hasFocus() && event.scrollY != 0 && boundsContains(event.x, event.y)) {
            commit();
            stepBy(event.scrollY > 0 ? 1 : -1);
            return Handled;
        }
        return Component::handleMouseScroll(event);
    }

    void NumberStepper::commit() {
        std::string text = _input->text();
        // A typographic minus, as the field may show it, is a minus
        for (std::size_t at = text.find("−"); at != std::string::npos; at = text.find("−")) {
            text.replace(at, 3, "-");
        }
        const auto first = text.find_first_not_of(" \t");
        const auto last  = text.find_last_not_of(" \t");
        if (first == std::string::npos) {
            refresh();
            return;
        }
        double      value = 0;
        const char *begin = text.data() + first;
        const char *end   = text.data() + last + 1;
        if (*begin == '+') {
            ++begin;
        }
        const auto [ptr, error] = std::from_chars(begin, end, value);
        if (error != std::errc{} || ptr != end || !std::isfinite(value)) {
            refresh();
            return;
        }
        updateValue(value);
    }

    void NumberStepper::refresh() {
        const std::string text = valueString();
        if (_input->text() != text) {
            _input->text(text);
            _input->textDisplay()->caret(static_cast<unsigned int>(text.size())); // Digits: one UTF-16 unit each
        }
        _decrement->enabled(_enabled && _value > _min);
        _increment->enabled(_enabled && _value < _max);
    }

    void NumberStepper::enabledChanged() {
        Component::enabledChanged();
        _input->enabled(_enabled);
        refresh();
    }

    bool NumberStepper::takesEscape() const {
        return _enabled && _input->text() != valueString();
    }
}
