#include <algorithm>
#include <charconv>
#include <cmath>
#include <format>
#include "NumberStepper.hpp"

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
        TextField<NumberStepperSkin>("", false),
        _value(value) {
        tag("NumberStepper");

        if (onChangeCallback) {
            changed.subscribe(std::move(onChangeCallback));
        }

        // Leaving the field sets what was typed
        blurred.subscribe([this]() { commit(); });

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
            const auto keepAlive = weak_from_this().lock(); // A handler may remove this stepper from its parent
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

    double NumberStepper::scale() const {
        return std::pow(10.0, _decimals >= 0 ? _decimals : decimalsOf(_step));
    }

    void NumberStepper::stepBy(const int count) {
        if (!_enabled) {
            return;
        }
        // Rounded to the decimals shown, so that steps of 0.1 don't drift; a step they can't show (0.5 with no
        // decimals) would round back to where it started, so it is one of their units at least
        const double scale = this->scale();
        const double step  = std::max(_step, 1.0 / scale);
        updateValue(std::round((_value + count * step) * scale) / scale);
    }

    std::string NumberStepper::valueString() const {
        return std::format("{:.{}f}", _value, _decimals >= 0 ? _decimals : decimalsOf(_step));
    }

    MouseEventStatus NumberStepper::handleMouseScroll(const MouseEvent &event) {
        if (_enabled && hasFocus() && event.scrollY != 0 && boundsContains(event.x, event.y)) {
            const auto keepAlive = weak_from_this().lock(); // A handler may remove this stepper from its parent
            commit();
            stepBy(event.scrollY > 0 ? 1 : -1);
            return Handled;
        }
        return TextField::handleMouseScroll(event);
    }

    void NumberStepper::commit() {
        std::string typed = text();
        // A typographic minus, as the field may show it, is a minus
        for (std::size_t at = typed.find("−"); at != std::string::npos; at = typed.find("−")) {
            typed.replace(at, 3, "-");
        }
        const auto first = typed.find_first_not_of(" \t");
        const auto last  = typed.find_last_not_of(" \t");
        if (first == std::string::npos) {
            refresh();
            return;
        }
        double      value = 0;
        const char *begin = typed.data() + first;
        const char *end   = typed.data() + last + 1;
        if (*begin == '+') {
            ++begin;
        }
        const auto [ptr, error] = std::from_chars(begin, end, value);
        if (error != std::errc{} || ptr != end || !std::isfinite(value)) {
            refresh();
            return;
        }
        // The value is what the field shows
        const double scale = this->scale();
        updateValue(std::round(value * scale) / scale);
    }

    void NumberStepper::refresh() {
        const std::string shown = valueString();
        if (text() != shown) {
            updateText(shown);
            const auto end = static_cast<unsigned int>(shown.size()); // Digits: one UTF-16 unit each
            apply(_editing.select(end, end));
        }
        if (_skin) {
            _skin->setBounds(_enabled && _value > _min, _enabled && _value < _max);
        }
    }

    bool NumberStepper::fieldKey(const Key key, const Mod mod) {
        const int steps = mod.shift ? 10 : 1;
        switch (key) {
            case Key::UP:
                commit();
                stepBy(steps);
                return true;
            case Key::DOWN:
                commit();
                stepBy(-steps);
                return true;
            case Key::ENTER:
                commit();
                return true;
            case Key::ESCAPE:
                refresh();
                return true;
            default:
                return false;
        }
    }

    bool NumberStepper::fieldPress(const MouseEvent &event) {
        if (!_skin) {
            return false;
        }
        int x = 0;
        int y = 0;
        _skin->globalToLocal(x, y, static_cast<int>(event.windowX), static_cast<int>(event.windowY));
        const NumberStepperSkin::Part part = _skin->partAt(static_cast<float>(x), static_cast<float>(y));
        if (part == NumberStepperSkin::Part::None) {
            return false;
        }
        commit();
        stepBy(part == NumberStepperSkin::Part::Increment ? 1 : -1);
        return true;
    }

    void NumberStepper::skinChanged() {
        TextField::skinChanged();
        refresh();
    }

    void NumberStepper::enabledChanged() {
        TextField::enabledChanged();
        refresh();
    }

    bool NumberStepper::takesEscape() const {
        return _enabled && text() != valueString();
    }
}
