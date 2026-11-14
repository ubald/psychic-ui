#include <cmath>
#include "include/core/SkColor.h"
#include "psychic-ui/style/StyleManager.hpp"
#include "DefaultAvatarSkin.hpp"

namespace psychic_ui {
    namespace {
        // Hue offsets from the accent's, in degrees: with Pivotal's indigo (235°) they are indigo, azure, teal,
        // violet, magenta and vermilion
        constexpr float hueOffsets[]{0, -35, -75, 45, 95, 145};

        // How much of the tint the circle takes, over its background
        constexpr std::uint8_t tintAlpha = 0x38; // 22%
    }

    DefaultAvatarSkin::DefaultAvatarSkin() :
        AvatarSkinBase() {
        tag("DefaultAvatarSkin");
    }

    Color DefaultAvatarSkin::tint(const std::string_view name, const Palette &palette) {
        // FNV-1a: stable across runs and platforms, unlike std::hash
        std::uint32_t hash = 2166136261u;
        for (const char c: name) {
            hash = (hash ^ static_cast<unsigned char>(c)) * 16777619u;
        }
        float accent[3];
        SkColorToHSV(palette.accent.argb(), accent);
        float text[3];
        SkColorToHSV(palette.accentText.argb(), text);
        float hsv[3]{
            std::fmod(accent[0] + hueOffsets[hash % std::size(hueOffsets)] + 360.0f, 360.0f),
            text[1],
            text[2]
        };
        return Color::fromARGB(SkHSVToColor(0xFF, hsv));
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
        if (avatar && avatar->colorful() && styleManager()) {
            _tint = tint(avatar->name(), styleManager()->theme().palette);
            _initials->color(_tint);
        } else {
            _tint = Color{};
            _initials->color(computedStyle().color()); // The rules' color, which the label would inherit
        }
    }

    void DefaultAvatarSkin::drawChrome(SkCanvas *canvas, const paint::Box &box, const SkinState state) {
        AvatarSkinBase::drawChrome(canvas, box, state);
        if (_tint.argb() != 0) {
            paint::fill(canvas, box.outer, _tint.withAlpha(tintAlpha), computedStyle().antiAlias());
        }
    }
}
