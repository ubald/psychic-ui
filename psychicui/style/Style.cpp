#include <iostream>
#include "Style.hpp"

namespace psychicui {

    // Inheritable properties
    const std::vector<ColorProperty>   Style::colorInheritable  = {color};
    const std::vector<StringProperty>  Style::stringInheritable = {fontFamily};
    const std::vector<FloatProperty>   Style::floatInheritable  = {};
    const std::vector<IntProperty>     Style::intInheritable    = {fontSize, letterSpacing, lineHeight};
    const std::vector<BoolProperty>    Style::boolInheritable   = {};

    std::unique_ptr<Style> Style::dummyStyle = std::make_unique<Style>();

    Style::Style() {
    }

    Style::Style(const Style *fromStyle) {
        overlay(fromStyle);
    }

    void Style::overlay(const Style *style) {
        if (!style) {
            return;
        }

        // Unfortunately for now this has to be manual

        for (auto const &kv : style->_colorValues) {
            _colorValues[kv.first] = kv.second;
        }

        for (auto const &kv : style->_stringValues) {
            _stringValues[kv.first] = kv.second;
        }

        for (auto const &kv : style->_floatValues) {
            _floatValues[kv.first] = kv.second;
        }

        for (auto const &kv : style->_intValues) {
            _intValues[kv.first] = kv.second;
        }

        for (auto const &kv : style->_boolValues) {
            _boolValues[kv.first] = kv.second;
        }

    }

    void Style::overlayInheritable(const Style *style) {
        if (!style) {
            return;
        }

        // Unfortunately for now this has to be manual

        for (auto const &kv : style->_colorValues) {
            if (std::find(colorInheritable.begin(), colorInheritable.end(), kv.first) != colorInheritable.end()) {
                _colorValues[kv.first] = kv.second;
            }
        }

        for (auto const &kv : style->_stringValues) {
            if (std::find(stringInheritable.begin(), stringInheritable.end(), kv.first) != stringInheritable.end()) {
                _stringValues[kv.first] = kv.second;
            }
        }

        for (auto const &kv : style->_floatValues) {
            if (std::find(floatInheritable.begin(), floatInheritable.end(), kv.first) != floatInheritable.end()) {
                _floatValues[kv.first] = kv.second;
            }
        }

        for (auto const &kv : style->_intValues) {
            if (std::find(intInheritable.begin(), intInheritable.end(), kv.first) != intInheritable.end()) {
                _intValues[kv.first] = kv.second;
            }
        }

        for (auto const &kv : style->_boolValues) {
            if (std::find(boolInheritable.begin(), boolInheritable.end(), kv.first) != boolInheritable.end()) {
                _boolValues[kv.first] = kv.second;
            }
        }

    }

    void Style::defaults(const Style *style) {
        if (!style) {
            return;
        }

        // Unfortunately for now this has to be manual

        for (auto const &kv : style->_colorValues) {
            if (_colorValues.find(kv.first) == _colorValues.end()) {
                _colorValues[kv.first] = kv.second;
            }
        }

        for (auto const &kv : style->_stringValues) {
            if (_stringValues.find(kv.first) == _stringValues.end()) {
                _stringValues[kv.first] = kv.second;
            }
        }

        for (auto const &kv : style->_floatValues) {
            if (_floatValues.find(kv.first) == _floatValues.end()) {
                _floatValues[kv.first] = kv.second;
            }
        }

        for (auto const &kv : style->_intValues) {
            if (_intValues.find(kv.first) == _intValues.end()) {
                _intValues[kv.first] = kv.second;
            }
        }

        for (auto const &kv : style->_boolValues) {
            if (_boolValues.find(kv.first) == _boolValues.end()) {
                _boolValues[kv.first] = kv.second;
            }
        }
    }

    bool Style::operator==(const Style &other) const {
        return _colorValues == other._colorValues
               && _stringValues == other._stringValues
               && _boolValues == other._boolValues
               && _intValues == other._intValues;
    }

    bool Style::operator!=(const Style &other) const {
        return !(*this == other);
    }
}