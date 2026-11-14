#pragma once

#include "Theme.hpp"
#include "ubuntu-type.hpp"

namespace psychic_ui::themes {
    /**
     * Atom's One Dark, in Ubuntu
     */
    inline constexpr Theme oneDark{
        .name = "One Dark",
        .scheme = ColorScheme::Dark,
        .palette = {
            .base             = rgb(0x1B1D23),
            .surface          = rgb(0x21252B),
            .raised           = rgb(0x2C313A),
            .overlay          = rgb(0x21252B),
            .frame            = rgb(0x21252B),
            .page             = rgb(0x282C34),
            .hover            = rgb(0x2C313C),
            .line             = rgb(0x181A1F),
            .lineStrong       = rgb(0x3C414F),
            .text             = rgb(0xD7DAE0),
            .textSecondary    = rgb(0x9DA5B5),
            .textMuted        = rgb(0x6B7280),
            .accent           = rgb(0x0C83EB),
            .accentHover      = rgb(0x2D95F0),
            .accentText       = rgb(0x61AFEF),
            .accentSoft       = argb(0x330C83EB),
            .onAccent         = rgb(0xFFFFFF),
            .danger           = rgb(0xE06C75),
            .dangerText       = rgb(0xE06C75),
            .warning          = rgb(0xE5C07B),
            .warningText      = rgb(0xE5C07B),
            .success          = rgb(0x98C379),
            .scrollThumb      = rgb(0x3C414F),
            .scrollThumbHover = rgb(0x4B5263),
            .scrim            = argb(0x80000000),
            .switchOff        = rgb(0x2C313A),
        },
        .elevation = {
            .overlay        = {.color = argb(0x8C000000), .blur = 32, .offsetY = 12, .spread = -8},
            .overlayContact = {.color = argb(0x4D000000), .blur = 6, .offsetY = 2},
            .thumb          = {.color = argb(0x4D000000), .blur = 2, .offsetY = 1},
        },
        .type = ubuntuType,
    };
}
