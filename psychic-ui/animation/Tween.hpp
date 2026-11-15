#pragma once

#include <array>
#include <chrono>
#include <concepts>
#include <cstddef>
#include <optional>
#include "Timing.hpp"
#include "psychic-ui/style/Values.hpp"

namespace psychic_ui {

    /**
     * A value as up to four numbers that interpolate linearly: a float is one, a color four (premultiplied)
     */
    using Channels = std::array<float, 4>;

    /**
     * How a type becomes channels and back. A type with a specialization can be tweened (`Animated<T>`) and, as a
     * style property, transition.
     */
    template<class T>
    struct Interpolation;

    template<class T>
    concept Interpolable = requires(const T &value, const Channels &channels) {
        { Interpolation<T>::count } -> std::convertible_to<std::size_t>;
        { Interpolation<T>::channels(value) } -> std::same_as<Channels>;
        { Interpolation<T>::value(channels, value) } -> std::same_as<T>;
        { Interpolation<T>::compatible(value, value) } -> std::same_as<bool>;
    };

    template<>
    struct Interpolation<float> {
        static constexpr std::size_t count = 1;

        static Channels channels(float value) { return {value, 0.0f, 0.0f, 0.0f}; }

        static float value(const Channels &channels, float /*target*/) { return channels[0]; }

        static bool compatible(float /*from*/, float /*to*/) { return true; }
    };

    /**
     * Premultiplied, as CSS interpolates colors: a fade from transparent shows no other color on the way
     */
    template<>
    struct Interpolation<Color> {
        static constexpr std::size_t count = 4;

        static Channels channels(Color color);

        static Color value(const Channels &channels, Color target);

        static bool compatible(Color /*from*/, Color /*to*/) { return true; }
    };

    /**
     * Lengths of the same unit; `Auto`, or a change of unit, doesn't interpolate
     */
    template<>
    struct Interpolation<Length> {
        static constexpr std::size_t count = 1;

        static Channels channels(Length length) { return {length.value, 0.0f, 0.0f, 0.0f}; }

        static Length value(const Channels &channels, Length target) { return Length{channels[0], target.unit}; }

        static bool compatible(Length from, Length to) { return from.unit == to.unit && !to.isAuto(); }
    };

    /**
     * The motion of up to four channels toward a target, with a timing: along an easing curve for its duration, or
     * pulled by a spring until it settles. Time is a monotonic clock's reading (`Window::updateAnimations`).
     *
     * A retarget starts from the current value: a curve takes its whole duration again, a spring keeps its velocity.
     */
    class Tween {
    public:
        Tween() = default;

        explicit Tween(const Channels &value, std::size_t count = 4);

        /**
         * At rest on a value
         */
        void jump(const Channels &value);

        /**
         * Head to `target` from the current value, starting at `start`, or at the first `advance` without one. An
         * instant timing jumps.
         */
        void retarget(const Channels &target, const Timing &timing, std::optional<std::chrono::nanoseconds> start);

        /**
         * Move to where it is at `now`
         * @return Whether it still moves
         */
        bool advance(std::chrono::nanoseconds now);

        /**
         * Go to the target at once
         */
        void finish();

        const Channels &value() const { return _value; }

        const Channels &target() const { return _target; }

        const Channels &velocity() const { return _velocity; }

        bool running() const { return _running; }

        const Timing &timing() const { return _timing; }

        /**
         * A spring that went on this long is stopped where it should be
         */
        static constexpr std::chrono::nanoseconds maxSpringTime = std::chrono::seconds(10);

    private:
        std::size_t _count{4};
        Channels    _from{};     // The value when it was retargeted
        Channels    _initialVelocity{};
        Channels    _target{};
        Channels    _value{};
        Channels    _velocity{}; // Per second
        Timing      _timing{};
        float       _tolerance{0.0f}; // A spring settles within this of its target
        std::optional<std::chrono::nanoseconds> _start{};
        std::optional<std::chrono::nanoseconds> _last{};
        bool        _running{false};
    };
}
