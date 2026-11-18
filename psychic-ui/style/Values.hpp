#pragma once

#include <concepts>
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <type_traits>
#include "Atom.hpp"
#include "Color.hpp"
#include "Length.hpp"

/**
 * The types of the style properties: lengths, colors, enumerated values and interned strings.
 * Each is checked when the call is compiled, so a typo or a wrong kind of value is a compile error.
 */
namespace psychic_ui {

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
