#pragma once

#include "Theme.hpp"
#include "pivotal.hpp"

/**
 * P2 Desk: Pivotal made into a desktop tool that is friendly with touch screens (`.lavish/design-system-options.html`,
 * P2). Pivotal's colors (light on a white base), a denser type scale and controls, 4 px corners, flush panes. Its sizes
 * switch with the input mode: for a pointer they are the board's "P2 Desk as it was"; by touch, approach B's (40 px
 * rows and controls, text a step larger, icons 2 px larger), with approach C's hit slop as the floor.
 */
namespace psychic_ui::themes {
    /**
     * Desk's type: Inter, a half step under Pivotal's (body 11.5 px, inputs and menus 12, titles 13)
     */
    inline constexpr Typography deskType{
        .xxs  = {.size = 10, .leading = 14},
        .xs   = {.size = 11, .leading = 16},
        .sm   = {.size = 11.5f, .leading = 17},
        .base = {.size = 12, .leading = 18},
        .md   = {.size = 13, .leading = 18},
        .xl   = {.size = 16, .leading = 22},
        .xxl  = {.size = 18, .leading = 24},
    };

    /**
     * Desk's metrics: 4 px corners (3 on check boxes, badges and key caps), no page inset, 12 px gutters, the
     * board's bars and panes; for a pointer 18 / 22 / 24 px controls and 26 px rows, by touch 24 / 36 / 40 and 40.
     * Pivotal's motion.
     */
    inline constexpr Metrics deskMetrics{
        .radius = {
            .control = 4,
            .button  = 4,
            .surface = 6,
            .popup   = 8,
            .row     = 4,
            .page    = 0,
            .check   = 3,
            .small   = 3,
        },
        .gutter           = 12,
        .gutterCard       = 16,
        .frameInset       = 0,
        .barToolbar       = 34,
        .barHeader        = 38,
        .sidebarWidth     = 212,
        .menuWidth        = 184,
        .toastWidth       = 310,
        .scrollThumbInset = 2,
        .pointer = {
            .controlXs = 18,
            .controlSm = 22,
            .controlMd = 24,
            .rowSm     = 26,
            .rowMd     = 26,
        },
        .touch = {
            .controlXs = 24,
            .controlSm = 36,
            .controlMd = 40,
            .rowSm     = 40,
            .rowMd     = 40,
            .textStep  = 1,
            .hitSlop   = 2, // 40 px rows and controls reach 44; the set gives smaller controls more
            .iconStep  = 2,
        },
        .motion = pivotalDark.metrics.motion,
    };

    /**
     * Desk Dark: Pivotal Dark's colors, elevations and canvas at desk sizes
     */
    inline constexpr Theme deskDark{
        .name = "Desk Dark",
        .scheme = ColorScheme::Dark,
        .palette = pivotalDark.palette,
        .elevation = pivotalDark.elevation,
        .type = deskType,
        .metrics = deskMetrics,
    };

    /**
     * Desk Light: Pivotal Light's colors on a white base and page, its elevations and canvas, at desk sizes
     */
    inline constexpr Theme deskLight{
        .name = "Desk Light",
        .scheme = ColorScheme::Light,
        .palette = [] {
            Palette palette = pivotalLight.palette;
            palette.base = rgb(0xFFFFFF);
            palette.page = rgb(0xFFFFFF);
            return palette;
        }(),
        .elevation = pivotalLight.elevation,
        .type = deskType,
        .metrics = deskMetrics,
    };
}
