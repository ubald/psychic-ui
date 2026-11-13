#pragma once

#include <bitset>
#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <string_view>
#include "Values.hpp"

/**
 * Every style property, defined once.
 *
 * X(name, Type, Argument, default, inherited, layout)
 * - name: the getter and setter name, on Style and on Div
 * - Type: how the value is stored (and returned by the getter)
 * - Argument: what the setter takes; the detail:: wrappers reject the conversions that are mistakes
 *   ("false" to bool, 0 to a string, a bool to a number)
 * - default: the value when the property isn't set
 * - inherited: a Div takes its parent's value when no rule sets it (CSS inheritance)
 * - layout: Yoga reads it, so changing it means a new layout; otherwise it only affects painting
 *
 * Adding a property: add a line here; if `layout` is true, push it to Yoga in `Div::updateLayout` (Div.cpp);
 * if it is paint-only, read it where it's drawn. Everything else (Style and Div setters, storage, cascade,
 * inheritance, change detection) follows from this list.
 */
#define PSYCHIC_UI_STYLE_PROPERTIES(X) \
    /* Colors */ \
    X(color,                    Color,         Color,              argb(0xFF000000),       true,  false) \
    X(backgroundColor,          Color,         Color,              argb(0xFF000000),       false, false) \
    X(borderColor,              Color,         Color,              argb(0xFF000000),       false, false) \
    X(borderHorizontalColor,    Color,         Color,              argb(0xFF000000),       false, false) \
    X(borderVerticalColor,      Color,         Color,              argb(0xFF000000),       false, false) \
    X(borderLeftColor,          Color,         Color,              argb(0xFF000000),       false, false) \
    X(borderRightColor,         Color,         Color,              argb(0xFF000000),       false, false) \
    X(borderTopColor,           Color,         Color,              argb(0xFF000000),       false, false) \
    X(borderBottomColor,        Color,         Color,              argb(0xFF000000),       false, false) \
    X(selectionColor,           Color,         Color,              argb(0xFF000000),       false, false) \
    X(selectionBackgroundColor, Color,         Color,              argb(0xFF000000),       false, false) \
    /* Background of components drawn as an inset "well" (text inputs, some buttons) */ \
    X(contentBackgroundColor,   Color,         Color,              argb(0xFF000000),       false, false) \
    /* Text */ \
    X(fontFamily,               Atom,          detail::StringArg,  Atom{},                 true,  false) \
    X(fontSize,                 float,         detail::FloatArg,   12.0f,                  true,  false) \
    X(letterSpacing,            float,         detail::FloatArg,   0.0f,                   true,  false) \
    /* Auto is 1.5 times the font size; a percent is of the font size */ \
    X(lineHeight,               Length,        Length,             Auto,                   true,  false) \
    X(textAlign,                TextAlign,     TextAlign,          TextAlign::Start,       true,  false) \
    X(textAntiAlias,            bool,          detail::BoolArg,    false,                  false, false) \
    /* Painting */ \
    X(antiAlias,                bool,          detail::BoolArg,    false,                  false, false) \
    X(opacity,                  float,         detail::FloatArg,   1.0f,                   false, false) \
    /* A line around the border box, outlineOffset away from it (inside if negative), over the div and its children; */ \
    /* none at width 0 */ \
    X(outlineColor,             Color,         Color,              argb(0xFF000000),       false, false) \
    X(outlineWidth,             float,         detail::FloatArg,   0.0f,                   false, false) \
    X(outlineOffset,            float,         detail::FloatArg,   0.0f,                   false, false) \
    /* A blurred copy of the border box under the div, outside it, like CSS's box-shadow; none while transparent. */ \
    /* shadowBlur is the blur radius (twice the Gaussian's sigma, as in CSS) */ \
    X(shadowColor,              Color,         Color,              argb(0x00000000),       false, false) \
    X(shadowBlur,               float,         detail::FloatArg,   0.0f,                   false, false) \
    X(shadowOffsetX,            float,         detail::FloatArg,   0.0f,                   false, false) \
    X(shadowOffsetY,            float,         detail::FloatArg,   0.0f,                   false, false) \
    /* Radii: the most specific one set wins (corner, then side, then all) */ \
    X(borderRadius,             float,         detail::FloatArg,   0.0f,                   false, false) \
    X(borderRadiusTop,          float,         detail::FloatArg,   0.0f,                   false, false) \
    X(borderRadiusBottom,       float,         detail::FloatArg,   0.0f,                   false, false) \
    X(borderRadiusLeft,         float,         detail::FloatArg,   0.0f,                   false, false) \
    X(borderRadiusRight,        float,         detail::FloatArg,   0.0f,                   false, false) \
    X(borderRadiusTopLeft,      float,         detail::FloatArg,   0.0f,                   false, false) \
    X(borderRadiusTopRight,     float,         detail::FloatArg,   0.0f,                   false, false) \
    X(borderRadiusBottomLeft,   float,         detail::FloatArg,   0.0f,                   false, false) \
    X(borderRadiusBottomRight,  float,         detail::FloatArg,   0.0f,                   false, false) \
    /* Hidden divs are still laid out, they are only skipped when painting and hit testing */ \
    X(visible,                  bool,          detail::BoolArg,    true,                   false, false) \
    X(cursor,                   Cursor,        Cursor,             Cursor::Arrow,          true,  false) \
    /* Components: the name of the skin to hatch (see StyleManager::registerSkin), slider orientation */ \
    X(skin,                     Atom,          detail::StringArg,  Atom{},                 false, false) \
    X(orientation,              Orientation,   Orientation,        Orientation::Horizontal, false, false) \
    /* Space between children: gap sets both, rowGap is between rows (lines), columnGap between columns */ \
    X(gap,                      Length,        Length,             Length{},               false, true) \
    X(rowGap,                   Length,        Length,             Length{},               false, true) \
    X(columnGap,                Length,        Length,             Length{},               false, true) \
    /* Flexbox */ \
    X(direction,                Direction,     Direction,          Direction::Column,      false, true) \
    X(textDirection,            TextDirection, TextDirection,      TextDirection::Inherit, false, true) \
    X(justify,                  Justify,       Justify,            Justify::Start,         false, true) \
    X(alignContent,             Align,         Align,              Align::Start,           false, true) \
    X(alignItems,               Align,         Align,              Align::Stretch,         false, true) \
    X(alignSelf,                Align,         Align,              Align::Auto,            false, true) \
    X(wrap,                     Wrap,          Wrap,               Wrap::NoWrap,           false, true) \
    X(flex,                     float,         detail::FloatArg,   0.0f,                   false, true) \
    X(grow,                     float,         detail::FloatArg,   0.0f,                   false, true) \
    X(shrink,                   float,         detail::FloatArg,   1.0f,                   false, true) \
    X(basis,                    Length,        Length,             Auto,                   false, true) \
    /* Box */ \
    X(position,                 Position,      Position,           Position::Relative,     false, true) \
    X(display,                  Display,       Display,            Display::Flex,          false, true) \
    /* Also paint: anything but visible clips the children */ \
    X(overflow,                 Overflow,      Overflow,           Overflow::Visible,      false, true) \
    X(boxSizing,                BoxSizing,     BoxSizing,          BoxSizing::BorderBox,   false, true) \
    X(aspectRatio,              float,         detail::FloatArg,   0.0f,                   false, true) \
    X(left,                     Length,        Length,             Auto,                   false, true) \
    X(top,                      Length,        Length,             Auto,                   false, true) \
    X(right,                    Length,        Length,             Auto,                   false, true) \
    X(bottom,                   Length,        Length,             Auto,                   false, true) \
    X(width,                    Length,        Length,             Auto,                   false, true) \
    X(height,                   Length,        Length,             Auto,                   false, true) \
    /* Auto means no limit */ \
    X(minWidth,                 Length,        Length,             Auto,                   false, true) \
    X(minHeight,                Length,        Length,             Auto,                   false, true) \
    X(maxWidth,                 Length,        Length,             Auto,                   false, true) \
    X(maxHeight,                Length,        Length,             Auto,                   false, true) \
    /* Edges: the most specific one set wins (side, then horizontal/vertical, then all) */ \
    X(margin,                   Length,        Length,             Length{},               false, true) \
    X(marginHorizontal,         Length,        Length,             Length{},               false, true) \
    X(marginVertical,           Length,        Length,             Length{},               false, true) \
    X(marginLeft,               Length,        Length,             Length{},               false, true) \
    X(marginTop,                Length,        Length,             Length{},               false, true) \
    X(marginRight,              Length,        Length,             Length{},               false, true) \
    X(marginBottom,             Length,        Length,             Length{},               false, true) \
    X(padding,                  Length,        Length,             Length{},               false, true) \
    X(paddingHorizontal,        Length,        Length,             Length{},               false, true) \
    X(paddingVertical,          Length,        Length,             Length{},               false, true) \
    X(paddingLeft,              Length,        Length,             Length{},               false, true) \
    X(paddingTop,               Length,        Length,             Length{},               false, true) \
    X(paddingRight,             Length,        Length,             Length{},               false, true) \
    X(paddingBottom,            Length,        Length,             Length{},               false, true) \
    X(border,                   float,         detail::FloatArg,   0.0f,                   false, true) \
    X(borderHorizontal,         float,         detail::FloatArg,   0.0f,                   false, true) \
    X(borderVertical,           float,         detail::FloatArg,   0.0f,                   false, true) \
    X(borderLeft,               float,         detail::FloatArg,   0.0f,                   false, true) \
    X(borderTop,                float,         detail::FloatArg,   0.0f,                   false, true) \
    X(borderRight,              float,         detail::FloatArg,   0.0f,                   false, true) \
    X(borderBottom,             float,         detail::FloatArg,   0.0f,                   false, true)

namespace psychic_ui {

    /**
     * Identifies a style property, for the operations on several at once (masks, `Style::has`)
     */
    enum class Property : std::uint8_t {
        #define PSYCHIC_UI_X(name, Type, Argument, defaultValue, inherited, layout) name,
        PSYCHIC_UI_STYLE_PROPERTIES(PSYCHIC_UI_X)
        #undef PSYCHIC_UI_X
    };

    inline constexpr std::size_t propertyCount = 0
        #define PSYCHIC_UI_X(name, Type, Argument, defaultValue, inherited, layout) + 1
        PSYCHIC_UI_STYLE_PROPERTIES(PSYCHIC_UI_X)
        #undef PSYCHIC_UI_X
        ;

    constexpr std::size_t index(Property property) { return static_cast<std::size_t>(property); }

    /**
     * A set of properties
     */
    using PropertyMask = std::bitset<propertyCount>;

    constexpr PropertyMask mask(std::initializer_list<Property> properties) {
        PropertyMask m;
        for (auto property: properties) {
            m.set(index(property));
        }
        return m;
    }

    /**
     * Properties a Div takes from its parent when nothing sets them
     */
    inline constexpr PropertyMask inheritedProperties = [] {
        PropertyMask m;
        #define PSYCHIC_UI_X(name, Type, Argument, defaultValue, inherited, layout) if (inherited) { m.set(index(Property::name)); }
        PSYCHIC_UI_STYLE_PROPERTIES(PSYCHIC_UI_X)
        #undef PSYCHIC_UI_X
        return m;
    }();

    /**
     * Properties Yoga reads: changing one of them means a new layout
     */
    inline constexpr PropertyMask layoutProperties = [] {
        PropertyMask m;
        #define PSYCHIC_UI_X(name, Type, Argument, defaultValue, inherited, layout) if (layout) { m.set(index(Property::name)); }
        PSYCHIC_UI_STYLE_PROPERTIES(PSYCHIC_UI_X)
        #undef PSYCHIC_UI_X
        return m;
    }();

    /**
     * Properties that only change how a Div is painted
     */
    inline constexpr PropertyMask paintProperties = ~layoutProperties;

    /**
     * The property's name, as in the setters
     */
    std::string_view propertyName(Property property);

    /**
     * Every property's value, flat: copying one is a plain copy, nothing is allocated
     */
    struct StyleValues {
        #define PSYCHIC_UI_X(name, Type, Argument, defaultValue, inherited, layout) Type name{defaultValue};
        PSYCHIC_UI_STYLE_PROPERTIES(PSYCHIC_UI_X)
        #undef PSYCHIC_UI_X

        bool operator==(const StyleValues &) const = default;
    };

    namespace detail {
        constexpr float toValue(FloatArg argument) { return argument.value; }

        constexpr bool toValue(BoolArg argument) { return argument.value; }

        inline Atom toValue(StringArg argument) { return Atom{argument.value}; }

        template<class T>
        constexpr const T &toValue(const T &argument) { return argument; }
    }
}
