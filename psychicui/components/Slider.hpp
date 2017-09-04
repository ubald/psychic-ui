#pragma once

#include <sstream>
#include <iomanip>
#include <functional>
#include "../psychicui.hpp"
#include "../SkinnableComponent.hpp"
#include "../skins/SliderSkin.hpp"
#include "Label.hpp"

namespace psychicui {

    namespace internal {
        /**
         * Base Slider Class
         */
        class SliderBase : public SkinnableComponent<SliderSkin, internal::SliderBase> {
        public:
            explicit SliderBase(std::shared_ptr<SliderSkin> skin) :
                SkinnableComponent(std::move(skin)) {
            }

            virtual const float getValuePercent() const = 0;
            virtual void setValuePercent(float percent) = 0;
            virtual std::string valueString() const = 0;
        };
    }

    template<class T>
    class Slider : public internal::SliderBase {
        static_assert(std::is_arithmetic<T>::value, "Sliders can only use numeric types.");
    public:
        Slider(std::function<void(T)> onChange, std::shared_ptr<SliderSkin> skin);

        explicit Slider(std::shared_ptr<SliderSkin> skin) : Slider(nullptr, skin) {};

        explicit Slider(std::function<void(T)> onChange) : Slider(onChange, nullptr) {};

        explicit Slider() : Slider(nullptr, nullptr) {};

        Slider<T> *setRange(const T min, const T max) {
            _min = min;
            _max = max;
            return this;
        }

        const T min() const {
            return _min;
        }

        Slider<T> *setMin(const T min) {
            _min = min;
            return this;
        }

        const T max() const {
            return _max;
        }

        Slider<T> *setMax(const T max) {
            _max = max;
            return this;
        }

        const T value() const {
            return _value;
        }

        Slider<T> *setValue(const T value) {
            _value = std::max(_min, std::min(value, _max));
            _skin->setValue(getValuePercent());
            return this;
        }

        std::string valueString() const override ;

        const float getValuePercent() const override;
        void setValuePercent(const float percent) override;
        void onChange(std::function<void(T)> callback);

    protected:
        T                      _min{0};
        T                      _max{1};
        T                      _value{0};
        std::function<void(T)> _onChange{nullptr};
    };

    template<class T>
    Slider<T>::Slider(std::function<void(T)> onChange, std::shared_ptr<SliderSkin> skin) :
        internal::SliderBase(skin),
        _onChange(onChange) {
        setTag("Slider");
    }

    template<class T>
    const float Slider<T>::getValuePercent() const {
        return ((float) _value - (float) _min) / ((float) _max - (float) _min);
    }
    template<class T>
    void Slider<T>::setValuePercent(const float percent) {
        setValue((T) ((percent * ((float) _max - (float) _min)) + (float) _min));
    }

    template<class T>
    std::string Slider<T>::valueString() const {
        std::stringstream stream;
        stream << std::fixed << std::setprecision(3) << _value;
        return stream.str();
    }

    template<class T>
    void Slider<T>::onChange(std::function<void(T)> callback) {
        _onChange = callback;
    }
}

