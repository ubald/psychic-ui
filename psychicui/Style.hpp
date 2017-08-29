#pragma once

#include <map>
#include <SkTypeface.h>
#include <psychicui/psychicui.hpp>

#define PSYCHIC_STYLE_PROPERTY(type, values, name, defaultValue)                                                       \
public:                                                                                                                \
type getValue(values property) const {                                                                                 \
    try {                                                                                                              \
        return _##name##Values.at(property);                                                                           \
    } catch (const std::out_of_range &exception){                                                                      \
        return defaultValue;                                                                                           \
    }                                                                                                                  \
}                                                                                                                      \
void setValue(values property, type value) {                                                                           \
    _##name##Values[property] = value;                                                                                 \
}                                                                                                                      \
protected:                                                                                                             \
std::map<values, type> _##name##Values{};                                                                              \


namespace psychicui {
    enum ColorProperty {
        // General
            transparent,
        // Background
            windowBackgroundColor,
            panelDropShadowColor,
            popupBackgroundColor,
        // Button
            buttonColor,
            buttonOverColor,
            buttonDownColor,
        // Text
            color
    };

    enum StringProperty {
        fontFamily
    };

    enum BoolProperty {
        // Text
        antiAlias,
            textAntiAlias
    };

    enum IntProperty {
        // Text
            textSize,
            textLeading,
        // Components
            titleBarHeight,
            panelCornerRadius,
            panelDropShadowSize,
            buttonCornerRadius
    };

    class Style {
    public:
        static Style * getDefaultStyle();
        static sk_sp<SkTypeface> defaultFont;

        Style();

        sk_sp<SkTypeface> font{nullptr};

        void overlay(Style * style);

        bool operator==(const Style &other) const;
        bool operator!=(const Style &other) const;

        // Macro stuff, don't put anything below, it'll end up protected
    PSYCHIC_STYLE_PROPERTY(Color, ColorProperty, color, 0xFF000000);
    PSYCHIC_STYLE_PROPERTY(std::string, StringProperty, string, "");
    PSYCHIC_STYLE_PROPERTY(bool, BoolProperty, bool, false);
    PSYCHIC_STYLE_PROPERTY(int, IntProperty, int, 0);

    };
}
