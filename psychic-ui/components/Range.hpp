#pragma once

#include <cmath>
#include <numbers>
#include <sstream>
#include <iomanip>
#include <functional>
#include "psychic-ui/psychic-ui.hpp"
#include "psychic-ui/Div.hpp"
#include "psychic-ui/Component.hpp"
#include "../Skin.hpp"
#include "../utils/StringUtils.hpp"
#include "Label.hpp"
#include "../signals/Signal.hpp"
#include "../signals/Slot.hpp"

namespace psychic_ui {

    namespace internal {
        class RangeBase;
    }

    class RangeSkin : public Skin<internal::RangeBase> {
    protected:
        RangeSkin() : Skin<internal::RangeBase>() {
            tag("RangeSkin");
        }

    public:
        /**
         * The value changed, as a linear percentage (0 to 1)
         */
        virtual void setValue(float /*value*/) {};

        /**
         * The linear percentage (0 to 1, or beyond, which the range clamps) a press or drag at this point, in the
         * skin's coordinates, picks. The range handles the mouse; the skin only knows where its values are.
         */
        virtual float valueAt(int x, int y) const = 0;
    };

    namespace internal {
        /**
         * Base Range Class
         */
        class RangeBase : public Component<RangeSkin> {
        protected:
            /**
             * Pressing sets the value under the mouse, then dragging (anywhere in the window) follows it
             */
            RangeBase();

        public:
            virtual float getLinearPercentage() const = 0;
            virtual void setLinearPercentage(float percent) = 0;
            virtual std::string valueString() const = 0;

        protected:
            ScopedSlot _onMouseMove{}; // On the window, only while dragging

            /**
             * Set the value the skin has at this point, in window coordinates
             */
            void dragTo(int windowX, int windowY);

            void removedFromRender() override;
        };
    }

    enum class RangeMode {
        Linear,
        Logarithmic,
        LogarithmicInverse
    };

    template<class T>
    class Range : public internal::RangeBase {
        static_assert(std::is_arithmetic<T>::value, "Ranges can only use numeric types.");
    public:
        Range(T min, T max, std::function<void(T)> onChangeCallback);
        Range(T min, T max, T value, std::function<void(T)> onChangeCallback);
        Range(T value, std::function<void(T)> onChangeCallback);
        explicit Range(std::function<void(T)> onChangeCallback);
        Range();

        // region Range & Value

        RangeMode mode() const;

        template<class Self>
        Self &mode(this Self &self, RangeMode mode) {
            static_cast<Range &>(self)._mode = mode;
            return self;
        }

        /**
         * Set the minimum and maximum (swapped if reversed), clamping the value
         */
        template<class Self>
        Self &range(this Self &self, T min, T max) {
            static_cast<Range &>(self).updateRange(min, max);
            return self;
        }

        const T min() const;

        template<class Self>
        Self &min(this Self &self, T min) {
            static_cast<Range &>(self).updateMin(min);
            return self;
        }

        const T max() const;

        template<class Self>
        Self &max(this Self &self, T max) {
            static_cast<Range &>(self).updateMax(max);
            return self;
        }

        const T step() const;

        template<class Self>
        Self &step(this Self &self, T step) {
            static_cast<Range &>(self)._step = step;
            return self;
        }

        const T value() const;

        /**
         * Set the value, clamped to the range
         */
        template<class Self>
        Self &value(this Self &self, T value) {
            static_cast<Range &>(self).updateValue(value);
            return self;
        }

        // endregion

        std::string valueString() const override;

        float getLinearPercentage() const override;
        void setLinearPercentage(float percent) override;

        /**
         * The value changed, with the new value
         */
        Signal<T> changed{};
        PSYCHIC_UI_EVENT(onChange, changed)

    protected:
        RangeMode _mode{RangeMode::Linear};
        T         _min{0};
        T         _max{1};
        T         _step{0};
        T         _value{0};
        void checkRange();
        void updateRange(T min, T max);
        void updateMin(T min);
        void updateMax(T max);
        void updateValue(T value);
        float getPercentageFor(T value) const;
        float getLinearPercentageFor(T value) const;
        float nearestValidValue(T value, T step) const;
    };

    template<class T>
    Range<T>::Range(T min, T max, T value, std::function<void(T)> onChangeCallback) :
        internal::RangeBase(), _min(min), _max(max), _value(value) {
        tag("Range");
        if (onChangeCallback) {
            changed.subscribe(std::forward<std::function<void(T)>>(onChangeCallback));
        }

        _step = std::is_floating_point<T>::value ? 0.01f : 1;

        checkRange();

        mouseScrolled.subscribe(
            [this](const MouseEvent &event) {
                const double scrollY = event.scrollY;
                if (std::is_floating_point<T>::value) {
                    updateValue(_value + (scrollY * _step));
                } else {
                    // Event the smallest movement should yield a result
                    updateValue(_value + (std::ceil(scrollY) * _step));
                }
            }
        );
    }

    template<class T>
    Range<T>::Range(T min, T max, std::function<void(T)> onChangeCallback) :
        Range(min, max, static_cast<T>(0), onChangeCallback) {}

    template<class T>
    Range<T>::Range(T value, std::function<void(T)> onChangeCallback) :
        Range(static_cast<T>(0), static_cast<T>(1), value, onChangeCallback) {}

    template<class T>
    Range<T>::Range(std::function<void(T)> onChangeCallback) :
        Range(static_cast<T>(0), static_cast<T>(1), static_cast<T>(0), onChangeCallback) {}

    template<class T>
    Range<T>::Range() :
        Range(nullptr) {}

    template<class T>
    RangeMode Range<T>::mode() const {
        return _mode;
    }

    template<class T>
    void Range<T>::checkRange() {
        if (_min > _max) {
            std::swap(_min, _max);
        } else if (_min == _max) {
            _max += _step;
        }
        updateValue(_value); // Clamp to new limit
    }

    template<class T>
    void Range<T>::updateRange(const T min, const T max) {
        _min = min;
        _max = max;
        checkRange();
    }

    template<class T>
    const T Range<T>::min() const {
        return _min;
    }

    template<class T>
    void Range<T>::updateMin(const T min) {
        _min = std::fmin(min, _max - _step);
        updateValue(_value); // Clamp to new limit
    }

    template<class T>
    const T Range<T>::max() const {
        return _max;
    }

    template<class T>
    void Range<T>::updateMax(const T max) {
        _max = std::max(_min + _step, max);
        updateValue(_value); // Clamp to new limit
    }

    template<class T>
    const T Range<T>::step() const {
        return _step;
    }

    template<class T>
    const T Range<T>::value() const {
        return _value;
    }

    template<class T>
    void Range<T>::updateValue(const T value) {
        float v = std::max(_min, std::min(value, _max));
        if (v != _value) {
            _value = v;
            changed.emit(_value);
            if (_skin) {
                _skin->setValue(getLinearPercentage());
            }
        }
    }

    template<class T>
    float Range<T>::getLinearPercentage() const {
        return getLinearPercentageFor(_value);
    }

    template<class T>
    void Range<T>::setLinearPercentage(const float percent) {
        float v;
        switch (_mode) {
            case RangeMode::Logarithmic:
                v = std::log((percent * 9.0f) + 1.0f) * std::numbers::log10e_v<float>;
                break;
            case RangeMode::LogarithmicInverse:
                v = 1.0f - std::log(((1.0f - percent) * 9.0f) + 1.0f) * std::numbers::log10e_v<float>;
                break;
            default:
                v = percent;
                break;
        }
        v = (v * (static_cast<float>(_max) - static_cast<float >(_min))) + static_cast<float >(_min);
        updateValue(static_cast<T>(std::is_floating_point<T>::value ? v : std::round(v)));
    }

    template<class T>
    float Range<T>::getPercentageFor(const T value) const {
        if (_max - _min <= 0) {
            return 0.0f;
        }
        return ((float) value - (float) _min) / ((float) _max - (float) _min);
    }

    template<class T>
    float Range<T>::getLinearPercentageFor(const T value) const {
        float v = getPercentageFor(value);
        switch (_mode) {
            case RangeMode::Logarithmic:
                return ((float) std::pow(10, v) - 1.0f) / 9.0f;
            case RangeMode::LogarithmicInverse:
                return 1.0f - ((float) std::pow(10, 1.0f - v) - 1.0f) / 9.0f;
            default:
                return v;
        }
    }

    /**
     *  Returns the sum of the minimum with an integer multiple of <code>interval</code> that's
     *  closest to <code>value</code>, unless <code>value</code> is closer to the maximum limit,
     *  in which case the maximum is returned.
     *
     *  <p>If <code>interval</code> is equal to 0, the value is clipped to the minimum and maximum
     *  limits.</p>
     *
     *  <p>The valid values for a range are defined by the sum of the <code>minimum</code> property
     *  with multiples of the <code>interval</code> and also defined to be less than or equal to the
     *  <code>maximum</code> property.
     *
     *  The maximum need not be a multiple of <code>snapInterval</code>.</p>
     *
     *  <p>For example, if <code>minimum</code> is equal to 1, <code>maximum</code> is equal to 6,
     *  and <code>snapInterval</code> is equal to 2, the valid
     *  values for the Range are 1, 3, 5, 6.
     *
     *  Similarly, if <code>minimum</code> is equal to 2, <code>maximum</code> is equal to 9,
     *  and <code>snapInterval</code> is equal to 1.5, the valid
     *  values for the Range are 2, 3.5, 5, 6.5, 8, and 9.</p>
     *
     *  @param value The input value.
     *  @param interval The value of snapInterval or an integer multiple of snapInterval.
     *  @return The valid value that's closest to the input.
     */
    template<class T>
    float Range<T>::nearestValidValue(T value, T interval) const {
        if (interval == 0) {
            return std::max(_min, std::fmin(_max, value));
        }

        float maxValue = _max - _min;
        float scale    = 1.0f;
        float offset   = _min; // the offset_from 0.

        // If interval isn't an integer, there's a possibility that the floating point
        // approximation of value or value/interval will be slightly larger or smaller
        // than the real value.  This can lead to errors in calculations like
        // floor(value/interval)*interval, which one might expect to just equal value,
        // when value is an exact multiple of interval.  Not so if value=0.58 and
        // interval=0.01, in that case the calculation yields 0.57!  To avoid problems,
        // we scale by the implicit precision of the interval and then round.  For
        // example if interval=0.01, then we scale by 100.

        if (interval != std::round(interval)) {
            // calculate scale and compute new scaled values.
            auto parts = string_utils::split(std::to_string(1 + interval), '.');
            scale    = (float) std::pow(10, parts[1].size());
            maxValue *= scale;
            offset *= scale;
            interval = std::round(interval * scale);
            value    = std::round((value * scale) - offset);
        } else {
            value -= offset;
        }

        float lower      = std::max(0, std::floor(value / interval) * interval);
        float upper      = std::fmin(maxValue, std::floor((value + interval) / interval) * interval);
        float validValue = ((value - lower) >= ((upper - lower) / 2)) ? upper : lower;

        return (validValue + offset) / scale;
    }

    template<class T>
    std::string Range<T>::valueString() const {
        std::stringstream stream;
        stream << std::fixed << std::setprecision(3) << _value;
        return stream.str();
    }

}

