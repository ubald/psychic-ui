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

        /**
         * WCAG's relative luminance of an opaque color
         */
        double luminance(const Color color) {
            const auto channel = [](const std::uint32_t value) {
                const double linear = static_cast<double>(value & 0xFFu) / 255.0;
                return linear <= 0.04045 ? linear / 12.92 : std::pow((linear + 0.055) / 1.055, 2.4);
            };
            const std::uint32_t argb = color.argb();
            return (0.2126 * channel(argb >> 16)) + (0.7152 * channel(argb >> 8)) + (0.0722 * channel(argb));
        }

        double contrast(const Color a, const Color b) {
            const double lumA = luminance(a);
            const double lumB = luminance(b);
            return (std::max(lumA, lumB) + 0.05) / (std::min(lumA, lumB) + 0.05);
        }
    }

    PivotalAvatarSkin::PivotalAvatarSkin() :
        AvatarSkinBase() {
        tag("PivotalAvatarSkin");
    }

    Color PivotalAvatarSkin::tint(const std::string_view name) {
        // FNV-1a: stable across runs and platforms, unlike std::hash
        std::uint32_t hash = 2166136261u;
        for (const char character: name) {
            hash = (hash ^ static_cast<unsigned char>(character)) * 16777619u;
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
