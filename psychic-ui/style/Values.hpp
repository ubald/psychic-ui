#pragma once

#include <concepts>
#include <cstddef>
#include <cstdint>
#include <ostream>
#include <string>
#include <string_view>
#include <type_traits>
#include <psychic-color/spaces/Color.hpp>

/**
 * The types of the style properties: lengths, colors, enumerated values and interned strings.
 * Each is checked when the call is compiled, so a typo or a wrong kind of value is a compile error.
 */
namespace psychic_ui {

    // region Length

    /**
     * A length in pixels, in percent of the parent (0-100, like CSS), or `Auto`.
     * A plain number is pixels: `width(24)`. Percent and auto read like CSS: `width(50_pct)`, `width(Auto)`.
     */
    struct Length {
        enum class Unit : std::uint8_t { Px, Percent, Auto };

        float value{0.0f};
        Unit  unit{Unit::Px};

        constexpr Length() = default;

        constexpr Length(float value, Unit unit) : value(value), unit(unit) {}

        /**
         * Pixels. Any number but a bool, which is a mistake
         */
        template<class T>
        requires (std::is_arithmetic_v<T> && !std::same_as<T, bool>)
        constexpr Length(T px) : value(static_cast<float>(px)) {} // NOLINT(google-explicit-constructor): 24 means 24px

        static constexpr Length px(float value) { return Length{value, Unit::Px}; }

        static constexpr Length percent(float value) { return Length{value, Unit::Percent}; }

        constexpr bool isPx() const { return unit == Unit::Px; }

        constexpr bool isPercent() const { return unit == Unit::Percent; }

        constexpr bool isAuto() const { return unit == Unit::Auto; }

        /**
         * The length in pixels, given what a percent is relative to; `fallback` when auto
         */
        constexpr float resolve(float relativeTo, float fallback = 0.0f) const {
            switch (unit) {
                case Unit::Px:
                    return value;
                case Unit::Percent:
                    return relativeTo * value / 100.0f;
                default:
                    return fallback;
            }
        }

        constexpr bool operator==(const Length &) const = default;
    };

    /**
     * The `auto` length: `width(Auto)`, `margin(Auto)`
     */
    inline constexpr Length Auto{0.0f, Length::Unit::Auto};

    inline namespace literals {
        /**
         * Percent of the parent, 0-100 like CSS: `width(50_pct)`
         */
        constexpr Length operator ""_pct(long double value) { return Length::percent(static_cast<float>(value)); }

        constexpr Length operator ""_pct(unsigned long long value) { return Length::percent(static_cast<float>(value)); }

        /**
         * Pixels, for when it reads better than a plain number: `width(24_px)`
         */
        constexpr Length operator ""_px(long double value) { return Length::px(static_cast<float>(value)); }

        constexpr Length operator ""_px(unsigned long long value) { return Length::px(static_cast<float>(value)); }
    }

    std::ostream &operator<<(std::ostream &os, const Length &length);

    // endregion

    // region Color

    /**
     * A 32 bits ARGB color.
     * A bare integer doesn't convert: say what it holds with `rgb(0xFF0000)` (opaque) or `argb(0x80FF0000)`.
     * A psychic-color color (RGB, HSB, ...) converts to its opaque Color.
     */
    class Color {
    public:
        constexpr Color() = default;

        /**
         * From a psychic-color color, opaque
         */
        Color(const psychic_color::Color &color) : // NOLINT(google-explicit-constructor): theme colors are psychic-color's
            _argb(0xFF000000u | (color.getColor() & 0x00FFFFFFu)) {}

        static constexpr Color fromARGB(std::uint32_t argb) {
            Color c;
            c._argb = argb;
            return c;
        }

        constexpr std::uint32_t argb() const { return _argb; }

        constexpr std::uint8_t alpha() const { return static_cast<std::uint8_t>(_argb >> 24); }

        /**
         * The same color with another alpha, 0 (transparent) to 255 (opaque)
         */
        constexpr Color withAlpha(std::uint8_t alpha) const {
            return fromARGB((static_cast<std::uint32_t>(alpha) << 24) | (_argb & 0x00FFFFFFu));
        }

        constexpr bool operator==(const Color &) const = default;

    private:
        std::uint32_t _argb{0x00000000u};
    };

    /**
     * An opaque color from 0xRRGGBB
     */
    constexpr Color rgb(std::uint32_t rgb) { return Color::fromARGB(0xFF000000u | (rgb & 0x00FFFFFFu)); }

    /**
     * An opaque color from its components
     */
    constexpr Color rgb(std::uint8_t r, std::uint8_t g, std::uint8_t b) {
        return Color::fromARGB(0xFF000000u | (std::uint32_t{r} << 16) | (std::uint32_t{g} << 8) | std::uint32_t{b});
    }

    /**
     * A color from 0xAARRGGBB, alpha first like Skia
     */
    constexpr Color argb(std::uint32_t argb) { return Color::fromARGB(argb); }

    std::ostream &operator<<(std::ostream &os, const Color &color);

    // endregion

    // region Atom

    /**
     * An interned string, for the open sets of values (font family, skin name).
     * Copying and comparing are pointer operations, and only the first use of a spelling allocates.
     * Interned strings are never freed; like the rest of the UI, Atoms are for the UI thread only.
     */
    class Atom {
    public:
        Atom();
        explicit Atom(std::string_view string);

        const std::string &str() const { return *_string; }

        const char *c_str() const { return _string->c_str(); }

        bool empty() const { return _string->empty(); }

        bool operator==(const Atom &other) const { return _string == other._string; }

        bool operator==(std::string_view other) const { return *_string == other; }

    private:
        const std::string *_string;
    };

    std::ostream &operator<<(std::ostream &os, const Atom &atom);

    // endregion

    // region Enumerated values

    /**
     * Flex direction: the axis children are laid out along
     */
    enum class Direction : std::uint8_t { Column, ColumnReverse, Row, RowReverse };

    /**
     * Text (and layout) direction, CSS's `direction`
     */
    enum class TextDirection : std::uint8_t { Inherit, Ltr, Rtl };

    /**
     * Main axis distribution, CSS's `justify-content`
     */
    enum class Justify : std::uint8_t { Start, Center, End, SpaceBetween, SpaceAround, SpaceEvenly };

    /**
     * Cross axis alignment, CSS's `align-items`, `align-self` and `align-content`
     */
    enum class Align : std::uint8_t { Auto, Start, Center, End, Stretch, Baseline, SpaceBetween, SpaceAround, SpaceEvenly };

    enum class Position : std::uint8_t { Static, Relative, Absolute };

    enum class Wrap : std::uint8_t { NoWrap, Wrap, WrapReverse };

    enum class Overflow : std::uint8_t { Visible, Hidden, Scroll };

    enum class Display : std::uint8_t { Flex, None, Contents };

    enum class BoxSizing : std::uint8_t { BorderBox, ContentBox };

    enum class TextAlign : std::uint8_t { Start, End, Left, Right, Center, Justify };

    enum class Cursor : std::uint8_t { Arrow, IBeam, Crosshair, Hand, HResize, VResize };

    /**
     * Orientation of sliders and scroll bars
     */
    enum class Orientation : std::uint8_t { Horizontal, Vertical };

    // endregion

    // region Setter arguments

    namespace detail {
        /**
         * A bool, and only a bool: `visible("false")` or `visible(1)` don't compile
         */
        struct BoolArg {
            bool value;

            template<std::same_as<bool> B>
            constexpr BoolArg(B b) : value(b) {} // NOLINT(google-explicit-constructor)
        };

        /**
         * A number, but not a bool
         */
        struct FloatArg {
            float value;

            template<class T>
            requires (std::is_arithmetic_v<T> && !std::same_as<T, bool>)
            constexpr FloatArg(T v) : value(static_cast<float>(v)) {} // NOLINT(google-explicit-constructor)
        };

        /**
         * Text, but not 0 or nullptr (which would convert to a null `const char *`)
         */
        struct StringArg {
            std::string_view value;

            StringArg(const char *string) : value(string) {} // NOLINT(google-explicit-constructor)
            StringArg(const std::string &string) : value(string) {} // NOLINT(google-explicit-constructor)
            StringArg(std::string_view string) : value(string) {} // NOLINT(google-explicit-constructor)
            StringArg(const Atom &atom) : value(atom.str()) {} // NOLINT(google-explicit-constructor)
            StringArg(std::nullptr_t) = delete;

            template<std::integral I>
            StringArg(I) = delete;
        };
    }

    // endregion
}
