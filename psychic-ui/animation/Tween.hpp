#pragma once

#include <array>
#include <chrono>
#include <cstddef>
#include <optional>
#include "Timing.hpp"

namespace psychic_ui {

    /**
     * A value as up to four numbers that move together (`Interpolation<T>` turns values into channels and back)
     */
    using Channels = std::array<float, 4>;

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
        bool        _running{false};
    };
}
