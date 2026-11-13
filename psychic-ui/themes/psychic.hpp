#pragma once

#include "Theme.hpp"

namespace psychic_ui::themes {
    /**
     * Dark grays with a cyan accent
     */
    inline constexpr Theme psychic{
        .name = "Psychic",
        .palette = {
            .background    = argb(0xFF19181A),
            .surface       = argb(0xFF252426),
            .muted         = argb(0xFF313033),
            .control       = argb(0xFF3D3C40),
            .controlHover  = argb(0xFF595959),
            .separator     = argb(0xFF19181A),
            .text          = argb(0xFFA4A1AA),
            .textMuted     = argb(0xFF6C6A70),
            .accent        = argb(0xFF00FFEC),
            .onAccent      = argb(0xFF151515),
            .onAccentMuted = argb(0xFF505050),
            .danger        = argb(0xFFE06C75),
            .warning       = argb(0xFFE5C07B),
            .success       = argb(0xFF98C379),
        },
    };
}
