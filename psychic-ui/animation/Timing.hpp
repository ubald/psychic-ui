#pragma once

#include <chrono>
#include <iosfwd>

/**
 * How a value moves to a new one: a duration along an easing curve, or a spring.
 */
namespace psychic_ui {

    /**
     * Where a moving value is, as its offset from its target (which goes to 0), at some time since it started
     */
    struct Approach {
        float offset{0.0f};
        float velocity{0.0f}; // Of the offset, per second
        bool  settled{false}; // There: the value takes its target
    };

    /**
     * An easing curve: a cubic Bézier from (0, 0) to (1, 1) through two control points, like CSS's `cubic-bezier()`.
     * `at(x)` is how far along its way a value is (y) at a share of the duration (x, 0 to 1). y may leave 0-1 (an
     * overshoot); the control points' x may not.
     *
     *     Ease::Out                                            // A named curve, CSS's `ease-out`
     *     Ease{.x1 = 0.16f, .y1 = 1.0f, .x2 = 0.3f, .y2 = 1.0f}  // Any other, as cubic-bezier(0.16, 1, 0.3, 1)
     */
    struct Ease {
        float x1{0.0f};
        float y1{0.0f};
        float x2{1.0f};
        float y2{1.0f};

        /**
         * The eased progress at a share of the duration (clamped to 0-1)
         */
        [[nodiscard]] float at(float progress) const;

        /**
         * How fast the eased progress changes at a share of the duration (dy/dx, clamped to 0-1)
         */
        [[nodiscard]] float slope(float progress) const;

        /**
         * Along this curve for `duration` seconds from an offset of `offset0` at rest, where it is `seconds` later:
         * `offset0 × (1 − at(seconds / duration))`, settled once the duration is over
         */
        [[nodiscard]] Approach approach(float offset0, float seconds, float duration) const;

        constexpr bool operator==(const Ease &) const = default;

        // CSS's named curves
        static const Ease Linear;   // linear
        static const Ease Standard; // ease, CSS's default: a quick start, a long settle
        static const Ease In;       // ease-in
        static const Ease Out;      // ease-out
        static const Ease InOut;    // ease-in-out
    };

    inline constexpr Ease Ease::Linear{.x1 = 0.0f, .y1 = 0.0f, .x2 = 1.0f, .y2 = 1.0f};
    inline constexpr Ease Ease::Standard{.x1 = 0.25f, .y1 = 0.1f, .x2 = 0.25f, .y2 = 1.0f};
    inline constexpr Ease Ease::In{.x1 = 0.42f, .y1 = 0.0f, .x2 = 1.0f, .y2 = 1.0f};
    inline constexpr Ease Ease::Out{.x1 = 0.0f, .y1 = 0.0f, .x2 = 0.58f, .y2 = 1.0f};
    inline constexpr Ease Ease::InOut{.x1 = 0.42f, .y1 = 0.0f, .x2 = 0.58f, .y2 = 1.0f};

    /**
     * A damped spring pulling a value to its target: no duration, it settles when it gets there. Interrupted, it keeps
     * its speed, so a value that reverses half-way turns smoothly. Below a damping of 2√(stiffness × mass) it
     * overshoots and oscillates; at or above it, it doesn't.
     *
     *     Spring{.stiffness = 500, .damping = 30}
     */
    struct Spring {
        float stiffness{170.0f};
        float damping{26.0f};
        float mass{1.0f};

        /**
         * Its damping ratio: below 1 it overshoots
         */
        [[nodiscard]] float dampingRatio() const;

        /**
         * Where it is `t` seconds after it had an offset of `x0` and a velocity of `v0` (the closed form, so the frame
         * rate doesn't change the motion): settled once the offset is within `tolerance` and the velocity within
         * `tolerance` × max(ω, 1), or after `maxTime`
         */
        [[nodiscard]] Approach approach(float x0, float v0, float t, float tolerance) const;

        constexpr bool operator==(const Spring &) const = default;

        /**
         * A spring that went on this long is settled
         */
        static constexpr std::chrono::nanoseconds maxTime = std::chrono::seconds(10);

        static const Spring Gentle; // Settles in about half a second, without overshooting
        static const Spring Snappy; // Quick, with a slight overshoot
        static const Spring Bouncy; // Overshoots and swings back a few times
    };

    inline constexpr Spring Spring::Gentle{.stiffness = 170.0f, .damping = 26.0f, .mass = 1.0f};
    inline constexpr Spring Spring::Snappy{.stiffness = 500.0f, .damping = 30.0f, .mass = 1.0f};
    inline constexpr Spring Spring::Bouncy{.stiffness = 400.0f, .damping = 14.0f, .mass = 1.0f};

    /**
     * How a value moves to a new one: a duration along an easing curve, or a spring. The default timing, and a
     * duration of 0, are instant.
     *
     *     Timing{150ms}                 // CSS's default curve
     *     Timing{100ms, Ease::Out}
     *     Timing{Spring::Snappy}
     */
    class Timing {
    public:
        constexpr Timing() = default;

        template<class Rep, class Period>
        constexpr Timing(std::chrono::duration<Rep, Period> duration, Ease ease = Ease::Standard) : // NOLINT(google-explicit-constructor)
            _duration(std::chrono::duration_cast<std::chrono::nanoseconds>(duration)), _ease(ease) {}

        constexpr Timing(Spring spring) : // NOLINT(google-explicit-constructor)
            _spring(spring), _isSpring(true) {}

        /**
         * Whether values jump to their end: no duration and no spring
         */
        [[nodiscard]] constexpr bool instant() const { return !_isSpring && _duration <= std::chrono::nanoseconds::zero(); }

        [[nodiscard]] constexpr bool isSpring() const { return _isSpring; }

        [[nodiscard]] constexpr std::chrono::nanoseconds duration() const { return _duration; }

        [[nodiscard]] constexpr const Ease &ease() const { return _ease; }

        [[nodiscard]] constexpr const Spring &spring() const { return _spring; }

        /**
         * Whether a retarget keeps the velocity: a spring does, a curve starts from rest
         */
        [[nodiscard]] constexpr bool keepsVelocity() const { return _isSpring; }

        /**
         * Where a value moving with this timing is, `seconds` after it had an offset of `offset0` from its target and
         * a velocity of `velocity0`: along the curve (from rest, whatever the velocity) or pulled by the spring.
         * `tolerance` is how close a spring settles (a curve settles when its duration is over).
         */
        [[nodiscard]] Approach approach(float offset0, float velocity0, float seconds, float tolerance) const;

        constexpr bool operator==(const Timing &) const = default;

    private:
        std::chrono::nanoseconds _duration{0};
        Ease                     _ease{Ease::Standard};
        Spring                   _spring{};
        bool                     _isSpring{false};
    };

    std::ostream &operator<<(std::ostream &os, const Timing &timing);
}
