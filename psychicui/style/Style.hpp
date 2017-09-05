#pragma once

#include <cmath>
#include <unordered_map>
#include <vector>
#include <string>
#include <functional>
#include <SkTypeface.h>
#include "../psychicui.hpp"

#define PSYCHIC_STYLE_PROPERTY(type, values, name, defaultValue)                                                       \
public:                                                                                                                \
type get(values property) const {                                                                                      \
    try {                                                                                                              \
        return _##name##Values.at(property);                                                                           \
    } catch (const std::out_of_range &exception){                                                                      \
        return defaultValue;                                                                                           \
    }                                                                                                                  \
}                                                                                                                      \
type get(values property, type fallback) const {                                                                       \
    try {                                                                                                              \
        return _##name##Values.at(property);                                                                           \
    } catch (const std::out_of_range &exception){                                                                      \
        return fallback;                                                                                               \
    }                                                                                                                  \
}                                                                                                                      \
Style * set(values property, type value) {                                                                             \
    _##name##Values[property] = value;                                                                                 \
    if (_onChanged) {                                                                                                  \
        _onChanged();                                                                                                  \
    }                                                                                                                  \
    return this;                                                                                                       \
}                                                                                                                      \
bool has(values property) {                                                                                            \
    return _##name##Values.find(property) != _##name##Values.end();                                                    \
}                                                                                                                      \
protected:                                                                                                             \
static const std::vector<values> name##Inheritable;                                                                    \
std::unordered_map<values, type> _##name##Values{};                                                                    \


namespace psychicui {

    enum ColorProperty {
        color,
        backgroundColor,
        borderColor,
        borderHorizontalColor,
        borderLeftColor,
        borderRightColor,
        borderVerticalColor,
        borderTopColor,
        borderBottomColor
    };

    enum StringProperty {
        skin,
        fontFamily, textAlign, textJustify,
        position,
        justifyContent, direction, alignContent, alignItems, alignSelf,
        wrap, overflow
    };

    enum FloatProperty {
        opacity,
        flex, grow, shrink, basis,
        fontSize, letterSpacing, lineHeight,
        left, leftPercent, right, rightPercent, top, topPercent, bottom, bottomPercent,
        width, widthPercent, minWidth, minWidthPercent, maxWidth, maxWidthPercent,
        height, heightPercent, minHeight, minHeightPercent, maxHeight, maxHeightPercent,
        margin, marginHorizontal, marginLeft, marginRight, marginVertical, marginTop, marginBottom,
        padding, paddingHorizontal, paddingLeft, paddingRight, paddingVertical, paddingTop, paddingBottom,
        border, borderHorizontal, borderLeft, borderRight, borderVertical, borderTop, borderBottom,
        borderRadius, borderRadiusTop, borderRadiusBottom, borderRadiusLeft, borderRadiusRight,
        borderRadiusTopLeft, borderRadiusTopRight, borderRadiusBottomLeft, borderRadiusBottomRight
    };

    enum IntProperty {

    };

    enum BoolProperty {
        antiAlias,
        textAntiAlias,
        visible
    };

    class Style {
    public:
        /**
         * Dummy style is used when we don't want to disrupt the styling code by having to
         * check for nullptrs, we just return this style instead, it is pure garbage ;)
         */
        static std::unique_ptr<Style> dummyStyle;


        static const float Auto;

        Style();
        explicit Style(const Style *fromStyle);
        explicit Style(const std::function<void()> &onChanged);

        sk_sp<SkTypeface> font{nullptr};

        /**
         * Add all values from style onto this
         * @param style
         */
        Style * overlay(const Style *style);

        /**
         * Add all inheritable values from style onto this
         * @param style
         */
        Style * overlayInheritable(const Style *style);

        /**
         * Fill empty properties in this with values from style
         * @param style
         */
        Style * defaults(const Style *style);

        bool operator==(const Style &other) const;
        bool operator!=(const Style &other) const;

        void trace() const;

    protected:
        std::function<void()> _onChanged{nullptr};

        // Macro stuff, don't put anything below, it'll end up protected
    PSYCHIC_STYLE_PROPERTY(Color, ColorProperty, color, 0xFF000000);
    PSYCHIC_STYLE_PROPERTY(std::string, StringProperty, string, "");
    PSYCHIC_STYLE_PROPERTY(float, FloatProperty, float, nanf("undefined"));
    PSYCHIC_STYLE_PROPERTY(int, IntProperty, int, 0);
    PSYCHIC_STYLE_PROPERTY(bool, BoolProperty, bool, false);

        #ifdef DEBUG_STYLES
    public:
        std::vector<std::string> declarations{};
        #endif

    };
}
