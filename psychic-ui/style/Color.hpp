#pragma once

#include <cstdint>
#include <ostream>
#include <psychic-color/spaces/Color.hpp>

namespace psychic_ui {

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
            Color color;
            color._argb = argb;
            return color;
        }

        [[nodiscard]] constexpr std::uint32_t argb() const { return _argb; }

        [[nodiscard]] constexpr std::uint8_t alpha() const { return static_cast<std::uint8_t>(_argb >> 24); }

        /**
         * The same color with another alpha, 0 (transparent) to 255 (opaque)
         */
        [[nodiscard]] constexpr Color withAlpha(std::uint8_t alpha) const {
            return fromARGB((static_cast<std::uint32_t>(alpha) << 24) | (_argb & 0x00FFFFFFu));
        }

        /**
         * This color painted over another (source over): what shows where a translucent fill lies on `below`
         */
        [[nodiscard]] constexpr Color over(const Color &below) const {
            const std::uint32_t a          = alpha();
            const std::uint32_t belowShown = below.alpha() * (255u - a) / 255u; // What of the color below shows through
            const std::uint32_t outAlpha   = a + belowShown;
            if (outAlpha == 0) {
                return Color{};
            }
            const auto channel = [&](const int shift) {
                const std::uint32_t own = (_argb >> shift) & 0xFFu;
                const std::uint32_t b   = (below._argb >> shift) & 0xFFu;
                return ((own * a + b * belowShown + outAlpha / 2) / outAlpha) << shift;
            };
            return fromARGB((outAlpha << 24) | channel(16) | channel(8) | channel(0));
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
}
