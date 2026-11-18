#include <algorithm>
#include <cmath>
#include <cstdint>
#include <string_view>
#include "include/core/SkCanvas.h"
#include "psychic-ui/themes/palette.hpp"
#include "PivotalAvatarSkin.hpp"
#include "psychic-ui/components/Avatar.hpp"
#include "psychic-ui/paint/Box.hpp"
#include "psychic-ui/skins/base/AvatarSkinBase.hpp"
#include "psychic-ui/skins/base/SkinState.hpp"
#include "psychic-ui/style/Color.hpp"

namespace psychic_ui {
    namespace {
        constexpr Color white     = rgb(0xFFFFFF);
        constexpr Color nearBlack = rgb(0x16171A);

        // sRGB's transfer function and WCAG's luminance weights and contrast flare
        constexpr double srgbLinearMax   = 0.04045; // Below, the curve is a line
        constexpr double srgbLinearSlope = 12.92;
        constexpr double srgbOffset      = 0.055;
        constexpr double srgbScale       = 1.055; // 1 + the offset
        constexpr double srgbGamma       = 2.4;
        constexpr double redWeight       = 0.2126;
        constexpr double greenWeight     = 0.7152;
        constexpr double blueWeight      = 0.0722;
        constexpr double flare           = 0.05;

        // FNV-1a, 32 bits
        constexpr std::uint32_t fnvOffsetBasis = 2166136261u;
        constexpr std::uint32_t fnvPrime       = 16777619u;

        /**
         * WCAG's relative luminance of an opaque color
         */
        double luminance(const Color color) {
            const auto channel = [](const std::uint32_t value) {
                const double linear = static_cast<double>(value & Color::channelMax) / Color::channelMax;
                return linear <= srgbLinearMax ? linear / srgbLinearSlope
                                               : std::pow((linear + srgbOffset) / srgbScale, srgbGamma);
            };
            const std::uint32_t argb = color.argb();
            return (redWeight * channel(argb >> Color::redShift)) + (greenWeight * channel(argb >> Color::greenShift))
                   + (blueWeight * channel(argb));
        }

        double contrast(const Color a, const Color b) {
            const double lumA = luminance(a);
            const double lumB = luminance(b);
            return (std::max(lumA, lumB) + flare) / (std::min(lumA, lumB) + flare);
        }
    }

    PivotalAvatarSkin::PivotalAvatarSkin() :
        AvatarSkinBase() {
        tag("PivotalAvatarSkin");
    }

    Color PivotalAvatarSkin::tint(const std::string_view name) {
        // FNV-1a: stable across runs and platforms, unlike std::hash
        std::uint32_t hash = fnvOffsetBasis;
        for (const char character: name) {
            hash = (hash ^ static_cast<unsigned char>(character)) * fnvPrime;
        }
        return palette::pivotalSwatches.at(hash % palette::pivotalHueCount);
    }

    Color PivotalAvatarSkin::initialsOn(const Color fill) {
        return contrast(fill, white) >= contrast(fill, nearBlack) ? white : nearBlack;
    }

    void PivotalAvatarSkin::setInitials(const std::string &initials) {
        AvatarSkinBase::setInitials(initials);
        updateTint();
    }

    void PivotalAvatarSkin::styleUpdated() {
        AvatarSkinBase::styleUpdated();
        updateTint();
    }

    void PivotalAvatarSkin::updateTint() {
        const Avatar *avatar = component();
        if (avatar && avatar->colorful()) {
            _tint = tint(avatar->name());
            _initials->color(initialsOn(_tint));
        } else {
            _tint = Color{};
            _initials->color(computedStyle().color()); // The rules' color, which the label would inherit
        }
    }

    void PivotalAvatarSkin::drawChrome(SkCanvas *canvas, const paint::Box &box, const SkinState state) {
        AvatarSkinBase::drawChrome(canvas, box, state);
        if (_tint.argb() != 0) {
            paint::fill(canvas, box.outer, _tint, computedStyle().antiAlias()); // Over the hairline too
        }
    }
}
