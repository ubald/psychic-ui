#pragma once

#include <array>
#include <bitset>
#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <iosfwd>
#include <string_view>
#include "Values.hpp"
#include "psychic-ui/animation/Timing.hpp"

/**
 * Every style property, defined once.
 *
 * X(name, Type, Argument, default, inherited, layout, animatable)
 * - name: the getter and setter name, on Style and on Div
 * - Type: how the value is stored (and returned by the getter)
 * - Argument: what the setter takes; the detail:: wrappers reject the conversions that are mistakes
 *   ("false" to bool, 0 to a string, a bool to a number)
 * - default: the value when the property isn't set
 * - inherited: a Div takes its parent's value when no rule sets it (CSS inheritance)
 * - layout: Yoga reads it, so changing it means a new layout; otherwise it only affects painting
 * - animatable: a `transition` can blend it (a color, a number, a length of one unit); otherwise it changes at once.
 *   A transitioning layout property is pushed to Yoga, and laid out, every frame of its transition.
 *
 * Adding a property: add a line here; if `layout` is true, push it to Yoga in `Div::updateLayout` (Div.cpp);
 * if it is paint-only, read it where it's drawn. Everything else (Style and Div setters, storage, cascade,
 * inheritance, change detection) follows from this list.
 */
#define PSYCHIC_UI_STYLE_PROPERTIES(X) \
    /* Colors */ \
    X(color,                    Color,         Color,              argb(0xFF000000),       true,  false, true ) \
    X(backgroundColor,          Color,         Color,              argb(0xFF000000),       false, false, true ) \
    X(borderColor,              Color,         Color,              argb(0xFF000000),       false, false, true ) \
    X(borderHorizontalColor,    Color,         Color,              argb(0xFF000000),       false, false, true ) \
    X(borderVerticalColor,      Color,         Color,              argb(0xFF000000),       false, false, true ) \
    X(borderLeftColor,          Color,         Color,              argb(0xFF000000),       false, false, true ) \
    X(borderRightColor,         Color,         Color,              argb(0xFF000000),       false, false, true ) \
    X(borderTopColor,           Color,         Color,              argb(0xFF000000),       false, false, true ) \
    X(borderBottomColor,        Color,         Color,              argb(0xFF000000),       false, false, true ) \
    /* Selected text and what is under it, inherited so that a field's parts take its own */ \
    X(selectionColor,           Color,         Color,              argb(0xFF000000),       true,  false, true ) \
    X(selectionBackgroundColor, Color,         Color,              argb(0xFF000000),       true,  false, true ) \
    /* The text caret, CSS's caret-color; unset, it is in `color` */ \
    X(caretColor,               Color,         Color,              argb(0xFF000000),       true,  false, true ) \
    /* Background of components drawn as an inset "well" (text inputs, some buttons) */ \
    X(contentBackgroundColor,   Color,         Color,              argb(0xFF000000),       false, false, true ) \
    /* Text */ \
    X(fontFamily,               Atom,          detail::StringArg,  Atom{},                 true,  false, false) \
    X(fontSize,                 float,         detail::FloatArg,   12.0f,                  true,  false, true ) \
    X(letterSpacing,            float,         detail::FloatArg,   0.0f,                   true,  false, true ) \
    /* Auto is 1.5 times the font size; a percent is of the font size */ \
    X(lineHeight,               Length,        Length,             Auto,                   true,  false, true ) \
    X(textAlign,                TextAlign,     TextAlign,          TextAlign::Start,       true,  false, false) \
    /* How glyph edges are smoothed; glyphs are always placed to the subpixel */ \
    X(fontSmoothing,           FontSmoothing, FontSmoothing,      FontSmoothing::Grayscale, true, false, false) \
    /* One line too long for its box: cut after its last whole character, or before an ellipsis */ \
    X(textOverflow,             TextOverflow,  TextOverflow,       TextOverflow::Clip,     false, false, false) \
    /* Icons (Icon, paint::icon), drawn in `color`: the side of the square, and the stroke in units of a 16-unit */ \
    /* grid whatever the icon's (Pivotal's 1.3), so every icon has the same weight at the same size */ \
    X(iconSize,                 float,         detail::FloatArg,   14.0f,                  true,  false, true ) \
    X(iconStrokeWidth,          float,         detail::FloatArg,   1.3f,                   true,  false, true ) \
    /* Painting */ \
    X(antiAlias,               bool,          detail::BoolArg,    false,                  false, false, false) \
    X(opacity,                  float,         detail::FloatArg,   1.0f,                   false, false, true ) \
    /* Paint transforms, of the div and its subtree as painted, after layout: never laid out nor hit tested (a press */ \
    /* finds a div where its layout puts it). Moved by translateX/translateY (a percent is of its border box's width */ \
    /* or height), scaled by scale times scaleX/scaleY about its transformOriginX/transformOriginY (from its border */ \
    /* box's top left corner, a percent of its size: the center by default), like CSS's translate, scale and */ \
    /* transform-origin. Popups enter and leave with them. */ \
    X(translateX,               Length,        Length,             Length{},               false, false, true ) \
    X(translateY,               Length,        Length,             Length{},               false, false, true ) \
    X(scale,                    float,         detail::FloatArg,   1.0f,                   false, false, true ) \
    X(scaleX,                   float,         detail::FloatArg,   1.0f,                   false, false, true ) \
    X(scaleY,                   float,         detail::FloatArg,   1.0f,                   false, false, true ) \
    X(transformOriginX,         Length,        Length,             Length::percent(50),    false, false, true ) \
    X(transformOriginY,         Length,        Length,             Length::percent(50),    false, false, true ) \
    /* A line around the border box, outlineOffset away from it (inside if negative), over the div and its children; */ \
    /* none at width 0 */ \
    X(outlineColor,             Color,         Color,              argb(0xFF000000),       false, false, true ) \
    X(outlineWidth,             float,         detail::FloatArg,   0.0f,                   false, false, true ) \
    X(outlineOffset,            float,         detail::FloatArg,   0.0f,                   false, false, true ) \
    /* A blurred copy of the border box under the div, outside it, like CSS's box-shadow; none while transparent. */ \
    /* shadowBlur is the blur radius (twice the Gaussian's sigma, as in CSS), shadowSpread grows (or, negative, */ \
    /* shrinks) the copy before the blur. shadow2* is a second layer, under the first, as in a CSS shadow list. */ \
    X(shadowColor,              Color,         Color,              argb(0x00000000),       false, false, true ) \
    X(shadowBlur,               float,         detail::FloatArg,   0.0f,                   false, false, true ) \
    X(shadowOffsetX,            float,         detail::FloatArg,   0.0f,                   false, false, true ) \
    X(shadowOffsetY,            float,         detail::FloatArg,   0.0f,                   false, false, true ) \
    X(shadowSpread,             float,         detail::FloatArg,   0.0f,                   false, false, true ) \
    X(shadow2Color,             Color,         Color,              argb(0x00000000),       false, false, true ) \
    X(shadow2Blur,              float,         detail::FloatArg,   0.0f,                   false, false, true ) \
    X(shadow2OffsetX,           float,         detail::FloatArg,   0.0f,                   false, false, true ) \
    X(shadow2OffsetY,           float,         detail::FloatArg,   0.0f,                   false, false, true ) \
    X(shadow2Spread,            float,         detail::FloatArg,   0.0f,                   false, false, true ) \
    /* A grid of dots over the background, under the children, like a design tool's canvas: dotGridSpacing apart */ \
    /* (the first half a step in from the border box's top left corner), each a disc of dotGridRadius, clipped to */ \
    /* the box; none while transparent */ \
    X(dotGridColor,             Color,         Color,              argb(0x00000000),       false, false, true ) \
    X(dotGridSpacing,           float,         detail::FloatArg,   16.0f,                  false, false, true ) \
    X(dotGridRadius,            float,         detail::FloatArg,   1.0f,                   false, false, true ) \
    /* Radii: the most specific one set wins (corner, then side, then all) */ \
    X(borderRadius,             float,         detail::FloatArg,   0.0f,                   false, false, true ) \
    X(borderRadiusTop,          float,         detail::FloatArg,   0.0f,                   false, false, true ) \
    X(borderRadiusBottom,       float,         detail::FloatArg,   0.0f,                   false, false, true ) \
    X(borderRadiusLeft,         float,         detail::FloatArg,   0.0f,                   false, false, true ) \
    X(borderRadiusRight,        float,         detail::FloatArg,   0.0f,                   false, false, true ) \
    X(borderRadiusTopLeft,      float,         detail::FloatArg,   0.0f,                   false, false, true ) \
    X(borderRadiusTopRight,     float,         detail::FloatArg,   0.0f,                   false, false, true ) \
    X(borderRadiusBottomLeft,   float,         detail::FloatArg,   0.0f,                   false, false, true ) \
    X(borderRadiusBottomRight,  float,         detail::FloatArg,   0.0f,                   false, false, true ) \
    /* Hidden divs are still laid out, they are only skipped when painting and hit testing */ \
    X(visible,                  bool,          detail::BoolArg,    true,                   false, false, false) \
    X(cursor,                   Cursor,        Cursor,             Cursor::Arrow,          true,  false, false) \
    /* How far around its border box a press still reaches the div (window units), when it lands on nothing that */ \
    /* takes presses; the nearest center wins between overlapping ones (Window::handleMouseButtonChange) */ \
    X(hitSlop,                  float,         detail::FloatArg,   0.0f,                   false, false, false) \
    /* Components: the variant of the skin set's skin to hatch ("" for the default, see SkinRegistry), slider orientation */ \
    X(skin,                     Atom,          detail::StringArg,  Atom{},                 false, false, false) \
    X(orientation,              Orientation,   Orientation,        Orientation::Horizontal, false, false, false) \
    /* Which properties blend to their new values when they change, and how (see Transition); not inherited */ \
    X(transition,               Transition,    Transition,         Transition{},           false, false, false) \
    /* A container's children slide from where they were painted to where a new layout puts them, with this timing */ \
    /* (a paint offset, FLIP: Div::layoutOffsetX/Y); instant, the default, leaves them where they are laid out */ \
    X(layoutTransition,         Timing,        Timing,             Timing{},               false, false, false) \
    /* Space between children: gap sets both, rowGap is between rows (lines), columnGap between columns */ \
    X(gap,                      Length,        Length,             Length{},               false, true, true ) \
    X(rowGap,                   Length,        Length,             Length{},               false, true, true ) \
    X(columnGap,                Length,        Length,             Length{},               false, true, true ) \
    /* Flexbox */ \
    X(direction,                Direction,     Direction,          Direction::Column,      false, true, false) \
    X(textDirection,            TextDirection, TextDirection,      TextDirection::Inherit, false, true, false) \
    X(justify,                  Justify,       Justify,            Justify::Start,         false, true, false) \
    X(alignContent,             Align,         Align,              Align::Start,           false, true, false) \
    X(alignItems,               Align,         Align,              Align::Stretch,         false, true, false) \
    X(alignSelf,                Align,         Align,              Align::Auto,            false, true, false) \
    X(wrap,                     Wrap,          Wrap,               Wrap::NoWrap,           false, true, false) \
    X(flex,                     float,         detail::FloatArg,   0.0f,                   false, true, true ) \
    X(grow,                     float,         detail::FloatArg,   0.0f,                   false, true, true ) \
    X(shrink,                   float,         detail::FloatArg,   1.0f,                   false, true, true ) \
    X(basis,                    Length,        Length,             Auto,                   false, true, true ) \
    /* Box */ \
    X(position,                 Position,      Position,           Position::Relative,     false, true, false) \
    X(display,                  Display,       Display,            Display::Flex,          false, true, false) \
    /* Also paint: anything but visible clips the children */ \
    X(overflow,                 Overflow,      Overflow,           Overflow::Visible,      false, true, false) \
    X(boxSizing,                BoxSizing,     BoxSizing,          BoxSizing::BorderBox,   false, true, false) \
    X(aspectRatio,              float,         detail::FloatArg,   0.0f,                   false, true, true ) \
    X(left,                     Length,        Length,             Auto,                   false, true, true ) \
    X(top,                      Length,        Length,             Auto,                   false, true, true ) \
    X(right,                    Length,        Length,             Auto,                   false, true, true ) \
    X(bottom,                   Length,        Length,             Auto,                   false, true, true ) \
    X(width,                    Length,        Length,             Auto,                   false, true, true ) \
    X(height,                   Length,        Length,             Auto,                   false, true, true ) \
    /* Auto means no limit */ \
    X(minWidth,                 Length,        Length,             Auto,                   false, true, true ) \
    X(minHeight,                Length,        Length,             Auto,                   false, true, true ) \
    X(maxWidth,                 Length,        Length,             Auto,                   false, true, true ) \
    X(maxHeight,                Length,        Length,             Auto,                   false, true, true ) \
    /* Edges: the most specific one set wins (side, then horizontal/vertical, then all) */ \
    X(margin,                   Length,        Length,             Length{},               false, true, true ) \
    X(marginHorizontal,         Length,        Length,             Length{},               false, true, true ) \
    X(marginVertical,           Length,        Length,             Length{},               false, true, true ) \
    X(marginLeft,               Length,        Length,             Length{},               false, true, true ) \
    X(marginTop,                Length,        Length,             Length{},               false, true, true ) \
    X(marginRight,              Length,        Length,             Length{},               false, true, true ) \
    X(marginBottom,             Length,        Length,             Length{},               false, true, true ) \
    X(padding,                  Length,        Length,             Length{},               false, true, true ) \
    X(paddingHorizontal,        Length,        Length,             Length{},               false, true, true ) \
    X(paddingVertical,          Length,        Length,             Length{},               false, true, true ) \
    X(paddingLeft,              Length,        Length,             Length{},               false, true, true ) \
    X(paddingTop,               Length,        Length,             Length{},               false, true, true ) \
    X(paddingRight,             Length,        Length,             Length{},               false, true, true ) \
    X(paddingBottom,            Length,        Length,             Length{},               false, true, true ) \
    X(border,                   float,         detail::FloatArg,   0.0f,                   false, true, true ) \
    X(borderHorizontal,         float,         detail::FloatArg,   0.0f,                   false, true, true ) \
    X(borderVertical,           float,         detail::FloatArg,   0.0f,                   false, true, true ) \
    X(borderLeft,               float,         detail::FloatArg,   0.0f,                   false, true, true ) \
    X(borderTop,                float,         detail::FloatArg,   0.0f,                   false, true, true ) \
    X(borderRight,              float,         detail::FloatArg,   0.0f,                   false, true, true ) \
    X(borderBottom,             float,         detail::FloatArg,   0.0f,                   false, true, true )

namespace psychic_ui {

    /**
     * Identifies a style property, for the operations on several at once (masks, `Style::has`)
     */
    enum class Property : std::uint8_t {
        #define PSYCHIC_UI_X(name, Type, Argument, defaultValue, inherited, layout, animatable) name,
        PSYCHIC_UI_STYLE_PROPERTIES(PSYCHIC_UI_X)
        #undef PSYCHIC_UI_X
    };

    /**
     * Every property, in declaration order
     */
    inline constexpr std::array allProperties{
        #define PSYCHIC_UI_X(name, Type, Argument, defaultValue, inherited, layout, animatable) Property::name,
        PSYCHIC_UI_STYLE_PROPERTIES(PSYCHIC_UI_X)
        #undef PSYCHIC_UI_X
    };

    inline constexpr std::size_t propertyCount = allProperties.size();

    constexpr std::size_t index(Property property) { return static_cast<std::size_t>(property); }

    /**
     * A set of properties
     */
    using PropertyMask = std::bitset<propertyCount>;

    constexpr PropertyMask mask(std::initializer_list<Property> properties) {
        PropertyMask result;
        for (auto property: properties) {
            result.set(index(property));
        }
        return result;
    }

    /**
     * The `transition` style property: which properties blend from their old value to their new one when it changes,
     * and with what timing, like CSS's `transition`. A few timings, each for a set of properties:
     *
     *     manager->style("Button")
     *         .transition({Property::backgroundColor, Property::borderColor}, Timing{100ms})
     *         .transition(Property::outlineColor, Timing{150ms, Ease::Out});
     *
     * Like CSS's, it cascades as a whole: the rule that wins sets every transition of the div, the others' don't add
     * to it. Only animatable properties (see `PSYCHIC_UI_STYLE_PROPERTIES`) blend; the others change at once.
     */
    class Transition {
    public:
        /**
         * How many different timings one transition holds
         */
        static constexpr std::size_t capacity = 4;

        constexpr Transition() = default;

        Transition(std::initializer_list<Property> properties, Timing timing) {
            add(mask(properties), timing);
        }

        /**
         * These properties blend with this timing (replacing their earlier timing). Throws `std::length_error` past
         * `capacity` timings.
         */
        Transition &add(const PropertyMask &properties, const Timing &timing);

        /**
         * The timing of a property, nullptr when it doesn't transition
         */
        [[nodiscard]] const Timing *timing(Property property) const;

        /**
         * The properties with a timing
         */
        [[nodiscard]] PropertyMask properties() const;

        [[nodiscard]] bool empty() const { return _count == 0; }

        bool operator==(const Transition &) const = default;

    private:
        struct Entry {
            PropertyMask properties{};
            Timing       timing{};

            bool operator==(const Entry &) const = default;
        };

        std::array<Entry, capacity> _entries{};
        std::uint8_t                _count{0};
    };

    std::ostream &operator<<(std::ostream &os, const Transition &transition);

    /**
     * Properties a Div takes from its parent when nothing sets them
     */
    inline constexpr PropertyMask inheritedProperties = [] {
        PropertyMask result;
        #define PSYCHIC_UI_X(name, Type, Argument, defaultValue, inherited, layout, animatable) if (inherited) { result.set(index(Property::name)); }
        PSYCHIC_UI_STYLE_PROPERTIES(PSYCHIC_UI_X)
        #undef PSYCHIC_UI_X
        return result;
    }();

    /**
     * Properties Yoga reads: changing one of them means a new layout
     */
    inline constexpr PropertyMask layoutProperties = [] {
        PropertyMask result;
        #define PSYCHIC_UI_X(name, Type, Argument, defaultValue, inherited, layout, animatable) if (layout) { result.set(index(Property::name)); }
        PSYCHIC_UI_STYLE_PROPERTIES(PSYCHIC_UI_X)
        #undef PSYCHIC_UI_X
        return result;
    }();

    /**
     * Properties that only change how a Div is painted
     */
    inline constexpr PropertyMask paintProperties = ~layoutProperties;

    /**
     * Properties a `transition` can blend
     */
    inline constexpr PropertyMask animatableProperties = [] {
        PropertyMask result;
        #define PSYCHIC_UI_X(name, Type, Argument, defaultValue, inherited, layout, animatable) if (animatable) { result.set(index(Property::name)); }
        PSYCHIC_UI_STYLE_PROPERTIES(PSYCHIC_UI_X)
        #undef PSYCHIC_UI_X
        return result;
    }();

    /**
     * The property's name, as in the setters
     */
    std::string_view propertyName(Property property);

    /**
     * Every property's value, flat: copying one is a plain copy, nothing is allocated
     */
    struct StyleValues {
        #define PSYCHIC_UI_X(name, Type, Argument, defaultValue, inherited, layout, animatable) Type name{defaultValue};
        PSYCHIC_UI_STYLE_PROPERTIES(PSYCHIC_UI_X)
        #undef PSYCHIC_UI_X

        bool operator==(const StyleValues &) const = default;
    };

    namespace detail {
        constexpr float toValue(FloatArg argument) { return argument.value; }

        constexpr bool toValue(BoolArg argument) { return argument.value; }

        inline Atom toValue(StringArg argument) { return Atom{argument.value}; }

        // By value: a reference to its argument would dangle once the temporary it was given is gone
        template<class T>
        constexpr T toValue(T argument) { return argument; }
    }
}
