#include <iostream>
#include "Style.hpp"

namespace psychicui {
    // Load fonts before creating the default setStyle
//    sk_sp<SkTypeface> Style::defaultFont = SkTypeface::MakeFromFile("res/fonts/stan0755.ttf");
    sk_sp<SkTypeface> Style::defaultFont = SkTypeface::MakeFromFile("res/fonts/Ubuntu/Ubuntu-Light.ttf");

    // Create the default setStyle
    Style * Style::getDefaultStyle() {
        Style * style = new Style();
        // General
        style->setValue(transparent, 0x00000000);
        // Background
        style->setValue(windowBackgroundColor, 0xFF000000);
        style->setValue(panelDropShadowColor, 0xFF000000);
        style->setValue(popupBackgroundColor, 0xFF000000);
        // Button
        style->setValue(buttonColor, 0xFF313033);
        style->setValue(buttonOverColor, 0xFF00FF00);
        style->setValue(buttonDownColor, 0xFF0000FF);
        // Text
        style->setValue(textAntiAlias, true);
        style->setValue(textSize, 12);
        style->setValue(textLeading, 16);
        style->setValue(color, 0xFFFFFFFF);

        return style;
    };

    Style::Style() :
        font(defaultFont) {
    }

    void Style::overlay(Style * style) {
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

        for (auto const &kv : style->_boolValues) {
            _boolValues[kv.first] = kv.second;
        }

        for (auto const &kv : style->_intValues) {
            _intValues[kv.first] = kv.second;
        }
    }

    bool Style::operator==(const Style &other) const {
        return  _colorValues == other._colorValues
            && _stringValues == other._stringValues
            && _boolValues == other._boolValues
            && _intValues == other._intValues;
    }

    bool Style::operator!=(const Style &other) const {
        return !(*this == other);
    }
}