#include <algorithm>
#include <cmath>
#include <ostream>
#include "Tween.hpp"

namespace psychic_ui {

    // region Timing

    float Ease::at(const float progress) const {
        const float x = std::clamp(progress, 0.0f, 1.0f);
        if (x == 0.0f || x == 1.0f || (x1 == y1 && x2 == y2)) {
            return x; // The ends, and a linear curve, are exact
        }
        // B(t) = 3(1-t)²t·p1 + 3(1-t)t²·p2 + t³, as a polynomial in t
        const float cx = 3.0f * x1;
        const float bx = 3.0f * (x2 - x1) - cx;
        const float ax = 1.0f - cx - bx;
        const float cy = 3.0f * y1;
        const float by = 3.0f * (y2 - y1) - cy;
        const float ay = 1.0f - cy - by;
        const auto  curveX = [&](float t) { return ((ax * t + bx) * t + cx) * t; };
        const auto  slopeX = [&](float t) { return (3.0f * ax * t + 2.0f * bx) * t + cx; };

        // The t whose x is the progress: Newton's method, then bisection where the slope is too flat for it
        float t = x;
        for (int i = 0; i < 8; ++i) {
            const float error = curveX(t) - x;
            if (std::abs(error) < 1e-6f) {
                return ((ay * t + by) * t + cy) * t;
            }
            const float slope = slopeX(t);
            if (std::abs(slope) < 1e-6f) {
                break;
            }
            t -= error / slope;
        }
        float low  = 0.0f;
        float high = 1.0f;
        t = x;
        for (int i = 0; i < 32; ++i) {
            const float value = curveX(t);
            if (std::abs(value - x) < 1e-6f) {
                break;
            }
            (value < x ? low : high) = t;
            t = (low + high) / 2.0f;
        }
        return ((ay * t + by) * t + cy) * t;
    }

    float Spring::dampingRatio() const {
        return damping / (2.0f * std::sqrt(stiffness * mass));
    }

    std::ostream &operator<<(std::ostream &os, const Timing &timing) {
        if (timing.isSpring()) {
            const Spring &s = timing.spring();
            return os << "spring(" << s.stiffness << ", " << s.damping << ", " << s.mass << ")";
        }
        const Ease &e = timing.ease();
        return os << std::chrono::duration<double, std::milli>(timing.duration()).count() << "ms cubic-bezier("
                  << e.x1 << ", " << e.y1 << ", " << e.x2 << ", " << e.y2 << ")";
    }

    // endregion

    // region Interpolation

    Channels Interpolation<Color>::channels(const Color color) {
        const std::uint32_t argb  = color.argb();
        const float         alpha = static_cast<float>((argb >> 24) & 0xFF) / 255.0f;
        const auto          premultiplied = [alpha](std::uint32_t component) {
            return static_cast<float>(component & 0xFF) / 255.0f * alpha;
        };
        return {premultiplied(argb >> 16), premultiplied(argb >> 8), premultiplied(argb), alpha};
    }

    Color Interpolation<Color>::value(const Channels &channels, Color /*target*/) {
        // A spring may overshoot: back within the gamut
        const float alpha = std::clamp(channels[3], 0.0f, 1.0f);
        if (alpha <= 0.0f) {
            return Color{};
        }
        const auto component = [alpha](float premultiplied) {
            return static_cast<std::uint32_t>(std::lround(std::clamp(premultiplied / alpha, 0.0f, 1.0f) * 255.0f));
        };
        return Color::fromARGB(
            (static_cast<std::uint32_t>(std::lround(alpha * 255.0f)) << 24)
            | (component(channels[0]) << 16) | (component(channels[1]) << 8) | component(channels[2])
        );
    }

    // endregion

    // region Tween

    namespace {
        float seconds(const std::chrono::nanoseconds duration) {
            return std::chrono::duration<float>(duration).count();
        }

        /**
         * A damped spring's displacement from its target and its velocity, `t` seconds after it had `x0` and `v0`
         * (the closed form, so the frame rate doesn't change the motion)
         */
        void springAt(const Spring &spring, const float x0, const float v0, const float t, float &x, float &v) {
            const float mass   = std::max(spring.mass, 1e-4f);
            const float omega  = std::sqrt(std::max(spring.stiffness, 0.0f) / mass);
            const float zeta   = omega > 0.0f ? spring.damping / (2.0f * std::sqrt(spring.stiffness * mass)) : 1.0f;
            if (omega <= 0.0f) {
                x = 0.0f; // No stiffness: nothing pulls, so it's there
                v = 0.0f;
                return;
            }
            if (std::abs(zeta - 1.0f) < 1e-4f) {
                // Critically damped
                const float b = v0 + omega * x0;
                const float e = std::exp(-omega * t);
                x = e * (x0 + b * t);
                v = e * (b - omega * (x0 + b * t));
            } else if (zeta < 1.0f) {
                // Underdamped: it oscillates as it decays
                const float a  = zeta * omega;
                const float wd = omega * std::sqrt(1.0f - zeta * zeta);
                const float b  = (v0 + a * x0) / wd;
                const float e  = std::exp(-a * t);
                const float c  = std::cos(wd * t);
                const float s  = std::sin(wd * t);
                x = e * (x0 * c + b * s);
                v = e * ((-a * x0 + b * wd) * c + (-a * b - x0 * wd) * s);
            } else {
                // Overdamped: two decaying exponentials
                const float root = omega * std::sqrt(zeta * zeta - 1.0f);
                const float r1   = -zeta * omega + root;
                const float r2   = -zeta * omega - root;
                const float c2   = (v0 - r1 * x0) / (r2 - r1);
                const float c1   = x0 - c2;
                const float e1   = std::exp(r1 * t);
                const float e2   = std::exp(r2 * t);
                x = c1 * e1 + c2 * e2;
                v = c1 * r1 * e1 + c2 * r2 * e2;
            }
        }
    }

    Tween::Tween(const Channels &value, const std::size_t count) :
        _count(std::min<std::size_t>(count, 4)), _from(value), _target(value), _value(value) {}

    void Tween::jump(const Channels &value) {
        _from     = value;
        _target   = value;
        _value    = value;
        _velocity = {};
        _start    = std::nullopt;
        _last     = std::nullopt;
        _running  = false;
    }

    void Tween::retarget(const Channels &target, const Timing &timing, const std::optional<std::chrono::nanoseconds> start) {
        if (timing.instant()) {
            jump(target);
            return;
        }
        _from            = _value;
        _initialVelocity = timing.isSpring() ? _velocity : Channels{}; // A curve starts from rest
        _target          = target;
        _timing          = timing;
        _start           = start;
        _last            = start;
        _running         = true;

        // A spring settles within a thousandth of its way, or of a pixel or a color step
        float span = 0.0f;
        for (std::size_t i = 0; i < _count; ++i) {
            span = std::max(span, std::abs(_target[i] - _from[i]));
        }
        _tolerance = std::max(span * 1e-3f, 1e-4f);
    }

    bool Tween::advance(const std::chrono::nanoseconds now) {
        if (!_running) {
            return false;
        }
        if (!_start) {
            // Started between frames: it starts with this one
            _start = now;
            _last  = now;
            return true;
        }
        const float elapsed = seconds(now - *_start);
        if (elapsed < 0.0f) {
            return true;
        }
        const Channels previous = _value;

        bool done = false;
        if (_timing.isSpring()) {
            done = true;
            for (std::size_t i = 0; i < _count; ++i) {
                float x = 0.0f;
                float v = 0.0f;
                springAt(_timing.spring(), _from[i] - _target[i], _initialVelocity[i], elapsed, x, v);
                _value[i]    = _target[i] + x;
                _velocity[i] = v;
                const float omega = std::sqrt(_timing.spring().stiffness / std::max(_timing.spring().mass, 1e-4f));
                if (std::abs(x) > _tolerance || std::abs(v) > _tolerance * std::max(omega, 1.0f)) {
                    done = false;
                }
            }
            done = done || now - *_start >= maxSpringTime;
        } else {
            const float duration = seconds(_timing.duration());
            const float progress = duration > 0.0f ? elapsed / duration : 1.0f;
            done = progress >= 1.0f;
            const float eased = _timing.ease().at(progress);
            for (std::size_t i = 0; i < _count; ++i) {
                _value[i] = _from[i] + (_target[i] - _from[i]) * eased;
            }
            // How fast it moves now, for a spring that would take over
            const float dt = _last ? seconds(now - *_last) : 0.0f;
            for (std::size_t i = 0; i < _count; ++i) {
                _velocity[i] = dt > 0.0f ? (_value[i] - previous[i]) / dt : 0.0f;
            }
        }
        _last = now;

        if (done) {
            finish();
        }
        return _running;
    }

    void Tween::finish() {
        _value    = _target;
        _velocity = {};
        _running  = false;
    }

    // endregion
}
