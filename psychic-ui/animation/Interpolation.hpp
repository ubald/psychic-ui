#pragma once

#include <concepts>
#include <cstddef>
#include <optional>
#include "Tween.hpp"
#include "psychic-ui/style/Color.hpp"
#include "psychic-ui/style/Length.hpp"

namespace psychic_ui {

    /**
     * How a type becomes channels and back. A type with a specialization can be tweened (`Animated<T>`) and, as a
     * style property, transition.
     *
     * `target(from, toward)` is where a move from `from` toward `toward` ends, as `value(channels, target)` rebuilds it
     * from channels (a length's unit comes from the target): `toward`, or nothing when they don't interpolate (the move
     * jumps).
     */
    template<class T>
    struct Interpolation;

    template<class T>
    concept Interpolable = requires(const T &value, const Channels &channels) {
        { Interpolation<T>::count } -> std::convertible_to<std::size_t>;
        { Interpolation<T>::channels(value) } -> std::same_as<Channels>;
        { Interpolation<T>::value(channels, value) } -> std::same_as<T>;
        { Interpolation<T>::target(value, value) } -> std::same_as<std::optional<T>>;
    };

    template<>
    struct Interpolation<float> {
        static constexpr std::size_t count = 1;

        static Channels channels(float value) { return {value, 0.0f, 0.0f, 0.0f}; }

        static float value(const Channels &channels, float /*target*/) { return channels[0]; }

        static std::optional<float> target(float /*from*/, float toward) { return toward; }
    };

    /**
     * Premultiplied, as CSS interpolates colors: a fade from transparent shows no other color on the way
     */
    template<>
    struct Interpolation<Color> {
        static constexpr std::size_t count = 4;

        static Channels channels(Color color);

        static Color value(const Channels &channels, Color target);

        static std::optional<Color> target(Color /*from*/, Color toward) { return toward; }
    };

    /**
     * Lengths of the same unit, zero being zero in any unit (50% to 0 px ends at 0%, 0 px to 50% starts at 0%);
     * `Auto`, or another unit, doesn't interpolate
     */
    template<>
    struct Interpolation<Length> {
        static constexpr std::size_t count = 1;

        static Channels channels(Length length) { return {length.value, 0.0f, 0.0f, 0.0f}; }

        static Length value(const Channels &channels, Length target) { return Length{channels[0], target.unit}; }

        static std::optional<Length> target(Length from, Length toward) {
            if (from.isAuto() || toward.isAuto()) {
                return std::nullopt;
            }
            if (from.unit == toward.unit || from.value == 0.0f) {
                return toward;
            }
            if (toward.value == 0.0f) {
                return Length{0.0f, from.unit};
            }
            return std::nullopt;
        }
    };
}
