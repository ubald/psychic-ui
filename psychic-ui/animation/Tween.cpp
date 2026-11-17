#include <algorithm>
#include <cmath>
#include "Tween.hpp"

namespace psychic_ui {

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
    }

    Tween::Tween(const Channels &value, const std::size_t count) :
        _count(std::min<std::size_t>(count, 4)), _from(value), _target(value), _value(value) {}

    void Tween::jump(const Channels &value) {
        _from     = value;
        _target   = value;
        _value    = value;
        _velocity = {};
        _start    = std::nullopt;
        _running  = false;
    }

    void Tween::retarget(const Channels &target, const Timing &timing, const std::optional<std::chrono::nanoseconds> start) {
        if (timing.instant()) {
            jump(target);
            return;
        }
        _from            = _value;
        _initialVelocity = timing.keepsVelocity() ? _velocity : Channels{};
        _target          = target;
        _timing          = timing;
        _start           = start;
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
            return true;
        }
        const float elapsed = seconds(now - *_start);
        if (elapsed < 0.0f) {
            return true;
        }
        bool done = true;
        for (std::size_t i = 0; i < _count; ++i) {
            const Approach approach = _timing.approach(_from[i] - _target[i], _initialVelocity[i], elapsed, _tolerance);
            _value[i]    = _target[i] + approach.offset;
            _velocity[i] = approach.velocity;
            done         = done && approach.settled;
        }

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
