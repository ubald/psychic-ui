#pragma once

#include <map>
#include <vector>
#include <SkTypeface.h>
#include "../psychicui.hpp"

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
static const std::vector<values> name##Inheritable;                                                              \
std::map<values, type> _##name##Values{};                                                                              \


namespace psychicui {

    enum ColorProperty {
        color,
        backgroundColor,
        borderColor, borderLeftColor, borderRightColor, borderTopColor, borderBottomColor
    };

    enum StringProperty {
        fontFamily, textAlign, textJustify,
        overflow, overflowX, overflowY,
        position
    };

    enum FloatProperty {
        opacity
    };

    enum IntProperty {
        fontSize, letterSpacing, lineHeight,
        width, minWidth, maxWidth,
        height, minHeight, maxHeight,
        margin, marginLeft, marginRight, marginTop, marginBottom,
        padding, paddingLeft, paddingRight, paddingTop, paddingBottom,
        border, borderLeft, borderRight, borderTop, borderBottom,
        cornerRadius, cornerRadiusLeft, cornerRadiusRight, cornerRadiusTop, cornerRadiusBottom
    };

    enum BoolProperty {
        antiAlias,
        textAntiAlias
    };

    class Style {
    public:
        /**
         * Dummy style is used when we don't want to disrupt the styling code by having to
         * check for nullptrs, we just return this style instead, it is pure garbage ;)
         */
        static std::unique_ptr<Style> dummyStyle;

        Style();
        Style(const Style * fromStyle);

        sk_sp<SkTypeface> font{nullptr};

        /**
         * Add all values from style onto this
         * @param style
         */
        void overlay(const Style *style);

        /**
         * Add all inheritable values from style onto this
         * @param style
         */
        void overlayInheritable(const Style *style);

        /**
         * Fill empty properties in this with values from style
         * @param style
         */
        void defaults(const Style *style);

        bool operator==(const Style &other) const;
        bool operator!=(const Style &other) const;

        // Macro stuff, don't put anything below, it'll end up protected
    PSYCHIC_STYLE_PROPERTY(Color, ColorProperty, color, 0xFF000000);
    PSYCHIC_STYLE_PROPERTY(std::string, StringProperty, string, "");
    PSYCHIC_STYLE_PROPERTY(float, FloatProperty, float, 0.0f);
    PSYCHIC_STYLE_PROPERTY(int, IntProperty, int, 0);
    PSYCHIC_STYLE_PROPERTY(bool, BoolProperty, bool, false);

    };
}
