#include <cmath>
#include "WindowScale.hpp"

namespace psychic_ui {

    namespace {
        /**
         * How far from 1 a ratio of the density to the scale is taken as 1: a density measured from
         * sizes rounded to whole pixels is off by a little
         */
        constexpr float sameScaleTolerance = 0.01f;
    }

    WindowScale WindowScale::fromDisplay(const float pixelDensity, const float displayScale) {
        if (!(pixelDensity > 0.0f) || !(displayScale > 0.0f) || !std::isfinite(pixelDensity) || !std::isfinite(displayScale)) {
            return {};
        }
        const float ratio = pixelDensity / displayScale;
        if (std::abs(ratio - 1.0f) < sameScaleTolerance) {
            return {};
        }
        return {.logicalPerWindowUnit = ratio};
    }

    WindowScale WindowScale::fromFramebuffer(const int windowWidth, const int framebufferWidth, const float contentScale) {
        if (windowWidth <= 0 || framebufferWidth <= 0) {
            return {};
        }
        return fromDisplay(static_cast<float>(framebufferWidth) / static_cast<float>(windowWidth), contentScale);
    }

    float WindowScale::pixelRatio(const int framebufferWidth, const int logicalWidth) {
        return logicalWidth > 0 ? static_cast<float>(framebufferWidth) / static_cast<float>(logicalWidth) : 1.0f;
    }

    float WindowScale::toLogical(const float windowUnits) const {
        return windowUnits * logicalPerWindowUnit;
    }

    int WindowScale::toLogical(const int windowUnits) const {
        return static_cast<int>(std::lround(static_cast<float>(windowUnits) * logicalPerWindowUnit));
    }

    int WindowScale::toWindow(const int logical) const {
        return static_cast<int>(std::lround(static_cast<float>(logical) / logicalPerWindowUnit));
    }
}
