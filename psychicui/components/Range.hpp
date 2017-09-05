#pragma once

#include <cmath>
#include <sstream>
#include <iomanip>
#include <functional>
#include "../psychicui.hpp"
#include "../Component.hpp"
#include "../SkinnableComponent.hpp"
#include "../Skin.hpp"
#include "../utils/StringUtils.hpp"
#include "Label.hpp"

namespace psychicui {

    namespace internal {
        class RangeBase;
    }

    class RangeSkin : public Skin<internal::RangeBase> {
    protected:
        RangeSkin() : Skin<internal::RangeBase>() {}

    public:
        virtual void setValue(float value) = 0;
    };

    namespace internal {
        /**
         * Base Range Class
         */
        class RangeBase : public SkinnableComponent<RangeSkin> {
        protected:
            RangeBase() = default;

        public:
            virtual float getLinearPercentage() const = 0;
            virtual void setLinearPercentage(const float percent) = 0;
            virtual std::string valueString() const = 0;
        };
    }

    enum RangeMode {
        linear,
        logarithmic,
        logarithmicInverse
    };

    template<class T>
    class Range : public internal::RangeBase {
        static_assert(std::is_arithmetic<T>::value, "Ranges can only use numeric types.");
    public:
        explicit Range(std::function<void(T)> onChange);
        Range();

        // region Range & Value

        const RangeMode mode() const;
        Range<T> *setMode(RangeMode mode);
        Range<T> *setRange(T min, T max);
        const T min() const;
        Range<T> *setMin(T min);
        const T max() const;
        Range<T> *setMax(T max);
        const T step() const;
        Range<T> *setStep(T step);
        const T value() const;
        Range<T> *setValue(T value);

        // endregion

        std::string valueString() const override;

        float getLinearPercentage() const override;
        void setLinearPercentage(const float percent) override;

        Range<T> *onChange(std::function<void(T)> callback);

    protected:
        RangeMode              _mode{linear};
        T                      _min{0};
        T                      _max{1};
        T                      _step{0};
        T                      _value{0};
        std::function<void(T)> _onChange{nullptr};
        float getPercentageFor(T value) const;
        float getLinearPercentageFor(T value) const;
        float nearestValidValue(T value, T step) const;
        void onMouseScroll(int mouseX, int mouseY, double scrollX, double scrollY) override;
    };

    template<class T>
    Range<T>::Range(std::function<void(T)> onChange) :
        internal::RangeBase(),
        _onChange(onChange) {
        setTag("Range");
        //_mouseChildren = false; // !important
        _step = std::is_floating_point<T>::value ? 0.01f : 1;
    }

    template<class T>
    Range<T>::Range() :
        Range(nullptr) {}

    template<class T>
    const RangeMode Range<T>::mode() const {
        return _mode;
    }

    template<class T>
    Range<T> *Range<T>::setMode(RangeMode mode) {
        _mode = mode;
        return this;
    }

    template<class T>
    Range<T> *Range<T>::setRange(const T min, const T max) {
        _min = min;
        _max = max;
        return this;
    }

    template<class T>
    const T Range<T>::min() const {
        return _min;
    }

    template<class T>
    Range<T> *Range<T>::setMin(const T min) {
        _min = std::min(min, _max);
        setValue(_value); // Clamp to new limit
        return this;
    }

    template<class T>
    const T Range<T>::max() const {
        return _max;
    }

    template<class T>
    Range<T> *Range<T>::setMax(const T max) {
        _max = std::max(_min, max);
        setValue(_value); // Clamp to new limit
        return this;
    }

    template<class T>
    const T Range<T>::step() const {
        return _step;
    }

    template<class T>
    Range<T> *Range<T>::setStep(const T step) {
        _step = step;
        return this;
    }

    template<class T>
    const T Range<T>::value() const {
        return _value;
    }

    template<class T>
    Range<T> *Range<T>::setValue(const T value) {
        float v = std::max(_min, std::min(value, _max));
        if (v != _value) {
            _value = v;
            if (_onChange) {
                _onChange(_value);
            }
            _skin->setValue(getLinearPercentage());
        }
        return this;
    }

    template<class T>
    float Range<T>::getLinearPercentage() const {
        return getLinearPercentageFor(_value);
    }

    template<class T>
    void Range<T>::setLinearPercentage(const float percent) {
        float v;
        switch (_mode) {
            case logarithmic:
                v = std::log((percent * 9.0f) + 1.0f) * (float) M_LOG10E;
                break;
            case logarithmicInverse:
                v = 1.0f - std::log(((1.0f - percent) * 9.0f) + 1.0f) * (float) M_LOG10E;
                break;
            default:
                v = percent;
                break;
        }
        setValue((T) ((v * ((float) _max - (float) _min)) + (float) _min));
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
            case logarithmic:
                return ((float) std::pow(10, v) - 1.0f) / 9.0f;
            case logarithmicInverse:
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
            return std::max(_min, std::min(_max, value));
        }

        float maxValue = _max - _min;
        float scale    = 1;
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
            auto parts = split(std::to_string(1 + interval), '.');
            scale    = (float) std::pow(10, parts[1].size());
            maxValue *= scale;
            offset *= scale;
            interval = std::round(interval * scale);
            value    = std::round((value * scale) - offset);
        } else {
            value -= offset;
        }

        float lower      = std::max(0, std::floor(value / interval) * interval);
        float upper      = std::min(maxValue, std::floor((value + interval) / interval) * interval);
        float validValue = ((value - lower) >= ((upper - lower) / 2)) ? upper : lower;

        return (validValue + offset) / scale;
    }

template<class T>
std::string Range<T>::valueString() const {
    std::stringstream stream;
    stream << std::fixed << std::setprecision(3) << _value;
    return stream.str();
}

template<class T>
Range <T> *Range<T>::onChange(std::function<void(T)> callback) {
    _onChange = callback;
    return this;
}

template<class T>
void Range<T>::onMouseScroll(const int mouseX, const int mouseY, const double scrollX, const double scrollY) {
    std::cout << scrollY << std::endl;
    if ( std::is_floating_point<T>::value) {
        setValue(_value + (scrollY * _step));
    } else {
        // Event the smalles movement should yield a result
        setValue(_value + (std::ceil(scrollY) * _step));
    }
}

}

