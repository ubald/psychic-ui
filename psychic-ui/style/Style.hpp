#pragma once

#include <cmath>
#include <unordered_map>
#include <map>
#include <vector>
#include <string>
#include <functional>
#include <SkTypeface.h>
#include <map>
#include <iostream>
#include "psychic-ui/psychic-ui.hpp"

#define PSYCHIC_STYLE_PROPERTY(type, values, name, defaultValue)                                                       \
public:                                                                                                                \
type get(values property) const {                                                                                      \
    auto search = _##name##Values.find(property);                                                                      \
    if (search != _##name##Values.cend()) {                                                                             \
        return search->second;                                                                                         \
    } else {                                                                                                           \
        return defaultValue;                                                                                           \
    }                                                                                                                  \
}                                                                                                                      \
type get(values property, type fallback) const {                                                                       \
    auto search = _##name##Values.find(property);                                                                      \
    if (search != _##name##Values.cend()) {                                                                             \
        return search->second;                                                                                         \
    } else {                                                                                                           \
        return fallback;                                                                                               \
    }                                                                                                                  \
}                                                                                                                      \
Style * set(values property, type value) {                                                                             \
    auto search = _##name##Values.find(property);                                                                      \
    if (search == _##name##Values.cend() || search->second != value) {                                                  \
        _##name##Values[property] = value;                                                                             \
        if (_onChanged) {                                                                                              \
            _onChanged();                                                                                              \
        }                                                                                                              \
    }                                                                                                                  \
    return this;                                                                                                       \
}                                                                                                                      \
bool has(values property) {                                                                                            \
    return _##name##Values.find(property) != _##name##Values.cend();                                                    \
}                                                                                                                      \
protected:                                                                                                             \
std::unordered_map<values, type, std::hash<int>> _##name##Values{};                                                    \


namespace psychic_ui {

    class Div;

    class Cursor {
    public:
        static const int Arrow = 0;
        static const int IBeam = 1;
        static const int Crosshair = 2;
        static const int Hand = 3;
        static const int HResize = 4;
        static const int VResize = 5;
    };

    enum ColorProperty {
        // Custom
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
        // Yoga/Flex
        fontFamily, textAlign, textJustify,
        position,
        justifyContent, direction, alignContent, alignItems, alignSelf,
        wrap, overflow,
        // Custom
        skin,
        orientation // For sliders
    };

    enum FloatProperty {
        // Yoga/Flex
        flex, grow, shrink, basis,
        left, leftPercent, right, rightPercent, top, topPercent, bottom, bottomPercent,
        width, widthPercent, minWidth, minWidthPercent, maxWidth, maxWidthPercent,
        height, heightPercent, minHeight, minHeightPercent, maxHeight, maxHeightPercent,
        margin, marginHorizontal, marginLeft, marginRight, marginVertical, marginTop, marginBottom,
        padding, paddingHorizontal, paddingLeft, paddingRight, paddingVertical, paddingTop, paddingBottom,
        border, borderHorizontal, borderLeft, borderRight, borderVertical, borderTop, borderBottom,
        // Custom
        opacity,
        fontSize, letterSpacing, lineHeight,
        borderRadius, borderRadiusTop, borderRadiusBottom, borderRadiusLeft, borderRadiusRight,
        borderRadiusTopLeft, borderRadiusTopRight, borderRadiusBottomLeft, borderRadiusBottomRight,
    };

    enum IntProperty {
        // Custom
        cursor,
        gap
    };

    enum BoolProperty {
        // Custom
        antiAlias,
        textAntiAlias,
        multiline,
        visible
    };

    struct InheritableValues {
        const std::vector<ColorProperty>  colorInheritable;
        const std::vector<StringProperty> stringInheritable;
        const std::vector<FloatProperty>  floatInheritable;
        const std::vector<IntProperty>    intInheritable;
        const std::vector<BoolProperty>   boolInheritable;
        InheritableValues(
            std::vector<ColorProperty> colorInherit,
            std::vector<StringProperty> stringInherit,
            std::vector<FloatProperty> floatInherit,
            std::vector<IntProperty> intInherit,
            std::vector<BoolProperty> boolInherit
        ) : colorInheritable(colorInherit),
            stringInheritable(stringInherit),
            floatInheritable(floatInherit),
            intInheritable(intInherit),
            boolInheritable(boolInherit) {}

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
        Style *overlay(const Style *style);

        /**
         * Add all inheritable values from style onto this
         * @param style
         */
        Style *overlayInheritable(const Style *style, const Div *div);

        /**
         * Fill empty properties in this with values from style
         * @param style
         */
        Style *defaults(const Style *style);

        bool operator==(const Style &other) const;
        bool operator!=(const Style &other) const;

        void trace() const;

    protected:
        std::function<void()> _onChanged{nullptr};

        template<class T>
        inline static void doOverlay(const T &from, T &onto) {
            for (auto const &kv : from) {
                onto[kv.first] = kv.second;
            }
        }

        template<class T>
        inline static void doInheritance(const T &from, T &onto, const std::vector<typename T::key_type> &inheritable) {
            for (auto const &kv : from) {
                if (std::find(inheritable.cbegin(), inheritable.cend(), kv.first) != inheritable.cend()) {
                    onto[kv.first] = kv.second;
                }
            }
        }

        template<class T>
        inline static void doDefaults(const T &from, T &onto) {
            for (auto const &kv : from) {
                if (onto.find(kv.first) == onto.cend()) {
                    onto[kv.first] = kv.second;
                }
            }
        }

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
