#include <algorithm>
#include <cmath>
#include <cstdint>
#include "Interpolation.hpp"
#include "Tween.hpp"
#include "psychic-ui/style/Color.hpp"

namespace psychic_ui {

    namespace {
        // A channel's full value, as the float its byte becomes
        constexpr auto channelMax = static_cast<float>(Color::channelMax);
    }

    Channels Interpolation<Color>::channels(const Color color) {
        const std::uint32_t argb  = color.argb();
        const float         alpha = static_cast<float>((argb >> Color::alphaShift) & Color::channelMax) / channelMax;
        const auto          premultiplied = [alpha](std::uint32_t component) {
            return static_cast<float>(component & Color::channelMax) / channelMax * alpha;
        };
        return {premultiplied(argb >> Color::redShift), premultiplied(argb >> Color::greenShift), premultiplied(argb), alpha};
    }

    Color Interpolation<Color>::value(const Channels &channels, Color /*target*/) {
        // A spring may overshoot: back within the gamut
        const float alpha = std::clamp(channels[3], 0.0f, 1.0f);
        if (alpha <= 0.0f) {
            return Color{};
        }
        const auto component = [alpha](float premultiplied) {
            return static_cast<std::uint32_t>(std::lround(std::clamp(premultiplied / alpha, 0.0f, 1.0f) * channelMax));
        };
        return Color::fromARGB(
            (static_cast<std::uint32_t>(std::lround(alpha * channelMax)) << Color::alphaShift)
            | (component(channels[0]) << Color::redShift) | (component(channels[1]) << Color::greenShift)
            | component(channels[2])
        );
    }
}
