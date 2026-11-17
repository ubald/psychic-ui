#include <algorithm>
#include <cmath>
#include <cstdint>
#include "Interpolation.hpp"

namespace psychic_ui {

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
}
