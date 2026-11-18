#pragma once

#include <concepts>
#include <cstdint>
#include <ostream>
#include <type_traits>

namespace psychic_ui {

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
        constexpr Length(T pixels) : value(static_cast<float>(pixels)) {} // NOLINT(google-explicit-constructor): 24 means 24px

        static constexpr Length px(float value) { return Length{value, Unit::Px}; }

        static constexpr Length percent(float value) { return Length{value, Unit::Percent}; }

        [[nodiscard]] constexpr bool isPx() const { return unit == Unit::Px; }

        [[nodiscard]] constexpr bool isPercent() const { return unit == Unit::Percent; }

        [[nodiscard]] constexpr bool isAuto() const { return unit == Unit::Auto; }

        /**
         * The length in pixels, given what a percent is relative to; `fallback` when auto
         */
        [[nodiscard]] constexpr float resolve(float relativeTo, float fallback = 0.0f) const {
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

    /**
     * All of the parent, `100_pct`: `width(Full)`, `height(Full)`
     */
    inline constexpr Length Full{100.0f, Length::Unit::Percent};

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
}
