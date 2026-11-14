#pragma once

#include "Theme.hpp"
#include "ubuntu-type.hpp"

namespace psychic_ui::themes {
    /**
     * Atom's One Light, in Ubuntu
     */
    inline constexpr Theme oneLight{
        .name = "One Light",
        .scheme = ColorScheme::Light,
        .palette = {
            .base             = rgb(0xFAFAFA),
            .surface          = rgb(0xEAEAEB),
            .raised           = rgb(0xDBDBDC),
            .overlay          = rgb(0xFFFFFF),
            .frame            = rgb(0xEAEAEB),
            .page             = rgb(0xFAFAFA),
            .hover            = rgb(0xE2E2E3),
            .line             = rgb(0xDBDBDC),
            .lineStrong       = rgb(0xC5CAD3),
            .text             = rgb(0x2C313B),
            .textSecondary    = rgb(0x4F525D),
            .textMuted        = rgb(0x696C77),
            .accent           = rgb(0x4078F2),
            .accentHover      = rgb(0x3366D9),
            .accentText       = rgb(0x3366D9),
            .accentSoft       = argb(0x264078F2),
            .onAccent         = rgb(0xFFFFFF),
            .danger           = rgb(0xE45649),
            .dangerText       = rgb(0xCA1243),
            .warning          = rgb(0xC18401),
            .warningText      = rgb(0x986801),
            .success          = rgb(0x50A14F),
            .scrollThumb      = argb(0x24000000),
            .scrollThumbHover = argb(0x3D000000),
            .scrim            = argb(0x3D000000),
            .switchOff        = rgb(0xBDBDBE),
        },
        .elevation = {
            .overlay        = {.color = argb(0x29000000), .blur = 32, .offsetY = 12, .spread = -8},
            .overlayContact = {.color = argb(0x14000000), .blur = 6, .offsetY = 2},
            .thumb          = {.color = argb(0x33000000), .blur = 2, .offsetY = 1},
        },
        .type = ubuntuType,
    };
}
