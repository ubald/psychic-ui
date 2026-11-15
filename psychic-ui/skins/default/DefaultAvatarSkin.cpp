#include <algorithm>
#include <cmath>
#include "psychic-ui/themes/palette.hpp"
#include "DefaultAvatarSkin.hpp"

namespace psychic_ui {
    namespace {
        constexpr Color white     = rgb(0xFFFFFF);
        constexpr Color nearBlack = rgb(0x16171A);

        /**
         * WCAG's relative luminance of an opaque color
         */
        double luminance(const Color color) {
            const auto channel = [](const std::uint32_t value) {
                const double c = static_cast<double>(value & 0xFFu) / 255.0;
                return c <= 0.04045 ? c / 12.92 : std::pow((c + 0.055) / 1.055, 2.4);
            };
            const std::uint32_t argb = color.argb();
            return 0.2126 * channel(argb >> 16) + 0.7152 * channel(argb >> 8) + 0.0722 * channel(argb);
        }

        double contrast(const Color a, const Color b) {
            const double la = luminance(a);
            const double lb = luminance(b);
            return (std::max(la, lb) + 0.05) / (std::min(la, lb) + 0.05);
        }
    }

    DefaultAvatarSkin::DefaultAvatarSkin() :
        AvatarSkinBase() {
        tag("DefaultAvatarSkin");
    }

    Color DefaultAvatarSkin::tint(const std::string_view name) {
        // FNV-1a: stable across runs and platforms, unlike std::hash
        std::uint32_t hash = 2166136261u;
        for (const char c: name) {
            hash = (hash ^ static_cast<unsigned char>(c)) * 16777619u;
        }
        return palette::pivotalSwatches[hash % palette::pivotalHueCount];
    }

    Color DefaultAvatarSkin::initialsOn(const Color fill) {
        return contrast(fill, white) >= contrast(fill, nearBlack) ? white : nearBlack;
    }

    void DefaultAvatarSkin::setInitials(const std::string &initials) {
        AvatarSkinBase::setInitials(initials);
        updateTint();
    }

    void DefaultAvatarSkin::styleUpdated() {
        AvatarSkinBase::styleUpdated();
        updateTint();
    }

    void DefaultAvatarSkin::updateTint() {
        const Avatar *avatar = component();
        if (avatar && avatar->colorful()) {
            _tint = tint(avatar->name());
            _initials->color(initialsOn(_tint));
        } else {
            _tint = Color{};
            _initials->color(computedStyle().color()); // The rules' color, which the label would inherit
        }
    }

    void DefaultAvatarSkin::drawChrome(SkCanvas *canvas, const paint::Box &box, const SkinState state) {
        AvatarSkinBase::drawChrome(canvas, box, state);
        if (_tint.argb() != 0) {
            paint::fill(canvas, box.outer, _tint, computedStyle().antiAlias()); // Over the hairline too
        }
    }
}
