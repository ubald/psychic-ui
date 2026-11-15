#pragma once

#include "Theme.hpp"
#include "ubuntu-type.hpp"

namespace psychic_ui::themes {
    /**
     * Dark grays with a cyan accent, in Ubuntu
     */
    inline constexpr Theme psychic{
        .name = "Psychic",
        .scheme = ColorScheme::Dark,
        .palette = {
            .base             = rgb(0x19181A),
            .surface          = rgb(0x201F21),
            .raised           = rgb(0x313033),
            .overlay          = rgb(0x252426),
            .frame            = rgb(0x201F21),
            .page             = rgb(0x19181A),
            .hover            = rgb(0x2A292C),
            .line             = argb(0x14FFFFFF),
            .lineStrong       = argb(0x24FFFFFF),
            .text             = rgb(0xE8E6EC),
            .textSecondary    = rgb(0xA4A1AA),
            .textMuted        = rgb(0x7A777F),
            .accent           = rgb(0x00FFEC),
            .accentHover      = rgb(0x5CFFF3),
            .accentText       = rgb(0x00FFEC),
            .accentSoft       = argb(0x2400FFEC),
            .onAccent         = rgb(0x151515),
            .danger           = rgb(0xE06C75),
            .dangerText       = rgb(0xE06C75),
            .warning          = rgb(0xE5C07B),
            .warningText      = rgb(0xE5C07B),
            .success          = rgb(0x98C379),
            .scrollThumb      = argb(0x17FFFFFF),
            .scrollThumbHover = argb(0x29FFFFFF),
            .scrim            = argb(0x80000000),
            .switchOff        = rgb(0x313033),
            .canvasDot        = argb(0x24FFFFFF), // The strong hairline
        },
        .elevation = {
            .overlay        = {.color = argb(0x8C000000), .blur = 32, .offsetY = 12, .spread = -8},
            .overlayContact = {.color = argb(0x4D000000), .blur = 6, .offsetY = 2},
            .thumb          = {.color = argb(0x4D000000), .blur = 2, .offsetY = 1},
            .panel          = {.color = argb(0x73000000), .blur = 24, .offsetY = 8, .spread = -6},
            .panelContact   = {.color = argb(0x24FFFFFF), .spread = 1}, // The strong hairline
        },
        .type = ubuntuType,
    };
}
