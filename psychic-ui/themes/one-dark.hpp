#pragma once

#include "Theme.hpp"

namespace psychic_ui::themes {
    /**
     * Atom's One Dark: the default theme
     */
    inline constexpr Theme oneDark{
        .name = "One Dark",
        .palette = {
            .background    = argb(0xFF1B1D23),
            .surface       = argb(0xFF21252B),
            .muted         = argb(0xFF282C34),
            .control       = argb(0xFF383D48),
            .controlHover  = argb(0xFF3C414F),
            .separator     = argb(0xFF181A1F),
            .text          = argb(0xFF9DA5B5),
            .textMuted     = argb(0xFF5C6370),
            .accent        = argb(0xFF0C83EB),
            .onAccent      = argb(0xFF151515),
            .onAccentMuted = argb(0xFF505050),
            .danger        = argb(0xFFE06C75),
            .warning       = argb(0xFFE5C07B),
            .success       = argb(0xFF98C379),
        },
    };
}
