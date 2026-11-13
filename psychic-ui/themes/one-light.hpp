#pragma once

#include "Theme.hpp"

namespace psychic_ui::themes {
    /**
     * Atom's One Light
     */
    inline constexpr Theme oneLight{
        .name = "One Light",
        .palette = {
            .background    = argb(0xFFC5CAD3),
            .surface       = argb(0xFFD7DAE1),
            .muted         = argb(0xFFE8EAED),
            .control       = argb(0xFFEBECEF),
            .controlHover  = argb(0xFFF6F7F8),
            .separator     = argb(0xFFC5CAD3),
            .text          = argb(0xFF2C313B),
            .textMuted     = argb(0xFF696C77),
            .accent        = argb(0xFF5A8AED),
            .onAccent      = argb(0xFFFFFFFF),
            .onAccentMuted = argb(0xFFDCE6FB),
            .danger        = argb(0xFFE45649),
            .warning       = argb(0xFFC18401),
            .success       = argb(0xFF50A14F),
        },
    };
}
