#include <algorithm>
#include <chrono>
#include <cmath>
#include <ostream>
#include <ratio>
#include "Timing.hpp"

namespace psychic_ui {

    // region Ease

    namespace {
        /**
         * An easing curve's B(t) = 3(1-t)²t·p1 + 3(1-t)t²·p2 + t³ as polynomials in t, with the t whose x is a progress
         */
        struct Bezier {
            float cx, bx, ax;
            float cy, by, ay;

            explicit Bezier(const Ease &ease) :
                cx(3.0f * ease.x1), bx((3.0f * (ease.x2 - ease.x1)) - cx), ax(1.0f - cx - bx),
                cy(3.0f * ease.y1), by((3.0f * (ease.y2 - ease.y1)) - cy), ay(1.0f - cy - by) {}

            [[nodiscard]] float x(float t) const { return ((ax * t + bx) * t + cx) * t; }

            [[nodiscard]] float y(float t) const { return ((ay * t + by) * t + cy) * t; }

            [[nodiscard]] float dx(float t) const { return ((3.0f * ax * t + 2.0f * bx) * t) + cx; }

            [[nodiscard]] float dy(float t) const { return ((3.0f * ay * t + 2.0f * by) * t) + cy; }

            /**
             * The t whose x is the progress: Newton's method, then bisection where the slope is too flat for it
             */
            [[nodiscard]] float solve(float progress) const {
                float t = progress;
                for (int i = 0; i < 8; ++i) {
                    const float error = x(t) - progress;
                    if (std::abs(error) < 1e-6f) {
                        return t;
                    }
                    const float slope = dx(t);
                    if (std::abs(slope) < 1e-6f) {
                        break;
                    }
                    t -= error / slope;
                }
                float low  = 0.0f;
                float high = 1.0f;
                t = progress;
                for (int i = 0; i < 32; ++i) {
                    const float value = x(t);
                    if (std::abs(value - progress) < 1e-6f) {
                        break;
                    }
                    (value < progress ? low : high) = t;
                    t = (low + high) / 2.0f;
                }
                return t;
            }
        };

        bool linear(const Ease &ease) { return ease.x1 == ease.y1 && ease.x2 == ease.y2; }
    }

    float Ease::at(const float progress) const {
        const float x = std::clamp(progress, 0.0f, 1.0f);
        if (x == 0.0f || x == 1.0f || linear(*this)) {
            return x; // The ends, and a linear curve, are exact
        }
        const Bezier curve{*this};
        return curve.y(curve.solve(x));
    }

    float Ease::slope(const float progress) const {
        if (linear(*this)) {
            return 1.0f;
        }
        const Bezier curve{*this};
        float        t  = curve.solve(std::clamp(progress, 0.0f, 1.0f));
        float        dx = curve.dx(t);
        if (std::abs(dx) < 1e-6f) {
            // A vertical tangent, or both derivatives 0 at an end (Ease::Out at 0): the slope just beside it
            t  = t < 0.5f ? t + 1e-3f : t - 1e-3f;
            dx = curve.dx(t);
        }
        return curve.dy(t) / dx;
    }

    Approach Ease::approach(const float offset0, const float seconds, const float duration) const {
        const float progress = duration > 0.0f ? seconds / duration : 1.0f;
        if (progress >= 1.0f) {
            return {.offset = 0.0f, .velocity = 0.0f, .settled = true};
        }
        return {
            .offset   = offset0 * (1.0f - at(progress)),
            .velocity = -offset0 * slope(progress) / duration,
            .settled  = false,
        };
    }

    // endregion

    // region Spring

    namespace {
        /**
         * Its mass, kept above 0 so that the motion stays finite
         */
        float massOf(const Spring &spring) { return std::max(spring.mass, 1e-4f); }
    }

    float Spring::dampingRatio() const {
        return damping / (2.0f * std::sqrt(stiffness * massOf(*this)));
    }

    Approach Spring::approach(const float offset0, const float velocity0, const float t, const float tolerance) const {
        const float omega = std::sqrt(std::max(stiffness, 0.0f) / massOf(*this));
        if (omega <= 0.0f) {
            return {.offset = 0.0f, .velocity = 0.0f, .settled = true}; // No stiffness: nothing pulls, so it's there
        }
        const float zeta     = dampingRatio();
        float       x        = 0.0f;
        float       velocity = 0.0f;
        if (std::abs(zeta - 1.0f) < 1e-4f) {
            // Critically damped
            const float b     = velocity0 + (omega * offset0);
            const float decay = std::exp(-omega * t);
            x        = decay * (offset0 + b * t);
            velocity = decay * (b - omega * (offset0 + b * t));
        } else if (zeta < 1.0f) {
            // Underdamped: it oscillates as it decays
            const float a           = zeta * omega;
            const float dampedOmega = omega * std::sqrt(1.0f - (zeta * zeta));
            const float b           = (velocity0 + a * offset0) / dampedOmega;
            const float decay       = std::exp(-a * t);
            const float cosine      = std::cos(dampedOmega * t);
            const float sine        = std::sin(dampedOmega * t);
            x        = decay * (offset0 * cosine + b * sine);
            velocity = decay * ((-a * offset0 + b * dampedOmega) * cosine + (-a * b - offset0 * dampedOmega) * sine);
        } else {
            // Overdamped: two decaying exponentials
            const float root    = omega * std::sqrt((zeta * zeta) - 1.0f);
            const float rate1   = (-zeta * omega) + root;
            const float rate2   = (-zeta * omega) - root;
            const float weight2 = (velocity0 - rate1 * offset0) / (rate2 - rate1);
            const float weight1 = offset0 - weight2;
            const float decay1  = std::exp(rate1 * t);
            const float decay2  = std::exp(rate2 * t);
            x        = (weight1 * decay1) + (weight2 * decay2);
            velocity = (weight1 * rate1 * decay1) + (weight2 * rate2 * decay2);
        }
        const bool within = std::abs(x) <= tolerance && std::abs(velocity) <= tolerance * std::max(omega, 1.0f);
        return {.offset = x, .velocity = velocity, .settled = within || t >= std::chrono::duration<float>(maxTime).count()};
    }

    // endregion

    // region Timing

    Approach Timing::approach(const float offset0, const float velocity0, const float seconds, const float tolerance) const {
        if (_isSpring) {
            return _spring.approach(offset0, velocity0, seconds, tolerance);
        }
        return _ease.approach(offset0, seconds, std::chrono::duration<float>(_duration).count());
    }

    std::ostream &operator<<(std::ostream &os, const Timing &timing) {
        if (timing.isSpring()) {
            const Spring &s = timing.spring();
            return os << "spring(" << s.stiffness << ", " << s.damping << ", " << s.mass << ")";
        }
        const Ease &curve = timing.ease();
        return os << std::chrono::duration<double, std::milli>(timing.duration()).count() << "ms cubic-bezier("
                  << curve.x1 << ", " << curve.y1 << ", " << curve.x2 << ", " << curve.y2 << ")";
    }

    // endregion
}
