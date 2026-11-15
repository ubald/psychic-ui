#pragma once

#include "Theme.hpp"

/**
 * S5 Float, the sample skin set's themes (`.lavish/design-system-options.html`, round 3): rounded panels floating over
 * a dotted canvas, gentle two-layer shadows, neutral greys and a violet accent, light first.
 * The canvas is the `frame` color; panels are the `surface`. Typography and metrics are shared by both.
 */
namespace psychic_ui::themes {
    // The two themes' shared groups, to mix into others (`Theme{.palette = ..., .metrics = themes::floatMetrics}`)

    // A bit larger type than Pivotal's: 12 px body, 12.5 px inputs and rows
    inline constexpr Typography floatTypography{
        .xxs  = {10.5f, 14},
        .xs   = {11, 16},
        .sm   = {12, 18},
        .base = {12.5f, 19},
        .md   = {13.5f, 20},
        .xl   = {17, 24},
        .xxl  = {20, 28},
    };

    // Controls 20 / 28 / 30, rows 30 / 34; corners 7 for controls (no pills), 12 for popups, 14 for panels
    inline constexpr Sizes floatSizes{
        .controlXs = 20,
        .controlSm = 28,
        .controlMd = 30,
        .rowSm     = 30,
        .rowMd     = 34,
    };

    inline constexpr Metrics floatMetrics{
        .radius = {
            .control = 7,
            .button  = 7,
            .surface = 14,
            .popup   = 12,
            .row     = 7,
            .page    = 14,
            .check   = 4,
            .small   = 6,
        },
        .gutter       = 14,
        .gutterCard   = 14,
        .frameInset   = 10, // The canvas between the panels
        .barToolbar   = 42,
        .barHeader    = 46,
        .sidebarWidth = 200,
        .iconSm       = 13,
        .iconMd       = 15,
        .iconStroke   = 1.35f,
        .pointer      = floatSizes,
        .touch        = {
            .controlXs = 20,
            .controlSm = 28,
            .controlMd = 30,
            .rowSm     = 30,
            .rowMd     = 34,
            .hitSlop   = 8, // A 28 px control reaches 44 px for a finger
        },
    };

    /**
     * Float Light, the set's default: a soft grey canvas, white panels, near-black text, violet where you act
     */
    inline constexpr Theme floatLight{
        .name = "Float Light",
        .scheme = ColorScheme::Light,
        .palette = {
            .base             = rgb(0xFFFFFF),
            .surface          = rgb(0xFFFFFF),
            .raised           = rgb(0xF2F2F4),
            .overlay          = rgb(0xFFFFFF),
            .frame            = rgb(0xEDEDF0), // The canvas
            .page             = rgb(0xFFFFFF),
            .hover            = rgb(0xF5F5F7),
            .line             = argb(0x14141428), // rgba(20 20 40 / 8%)
            .lineStrong       = argb(0x24141428), // 14%
            .text             = rgb(0x1B1B1F),
            .textSecondary    = rgb(0x4A4A52),
            .textMuted        = rgb(0x74747C),
            .accent           = rgb(0x7357FF),
            .accentHover      = rgb(0x6244F0),
            .accentText       = rgb(0x5F43E6),
            .accentSoft       = argb(0x1F7357FF), // 12%: #efebff on white, the selection
            .onAccent         = rgb(0xFFFFFF),
            .danger           = rgb(0xD6304A),
            .dangerText       = rgb(0xBF2440),
            .warning          = rgb(0xB5660C),
            .warningText      = rgb(0x97540A),
            .success          = rgb(0x1F8A55),
            .scrollThumb      = argb(0x29141428), // 16%
            .scrollThumbHover = argb(0x42141428), // 26%
            .scrim            = argb(0x33141428), // 20%
            .switchOff        = rgb(0xD4D4DA),
        },
        .elevation = {
            // Menus: 0 12px 32px rgb(20 20 40 / 14%), 0 2px 6px rgb(20 20 40 / 6%)
            .overlay        = {.color = argb(0x24141428), .blur = 32, .offsetY = 12},
            .overlayContact = {.color = argb(0x0F141428), .blur = 6, .offsetY = 2},
            .thumb          = {.color = argb(0x33000000), .blur = 2, .offsetY = 1},
            // Panels: a wide soft fall-off over a 1 px ring (the board's 0 2px 6px middle layer folded into the first)
            .panel          = {.color = argb(0x14141428), .blur = 24, .offsetY = 8},
            .panelContact   = {.color = argb(0x0F141428), .spread = 1},
        },
        .type = floatTypography,
        .metrics = floatMetrics,
    };

    /**
     * Float Dark: a near-black canvas, charcoal panels lifted by a faint light ring, a lighter violet
     */
    inline constexpr Theme floatDark{
        .name = "Float Dark",
        .scheme = ColorScheme::Dark,
        .palette = {
            .base             = rgb(0x18181B),
            .surface          = rgb(0x232326),
            .raised           = rgb(0x2E2E33),
            .overlay          = rgb(0x29292D),
            .frame            = rgb(0x141416), // The canvas
            .page             = rgb(0x232326),
            .hover            = rgb(0x2B2B30),
            .line             = argb(0x14F0F0FF), // rgba(240 240 255 / 8%)
            .lineStrong       = argb(0x24F0F0FF), // 14%
            .text             = rgb(0xF4F4F6),
            .textSecondary    = rgb(0xC6C6CC),
            .textMuted        = rgb(0x93939B),
            .accent           = rgb(0x8A74FF),
            .accentHover      = rgb(0x9C89FF),
            .accentText       = rgb(0xA796FF),
            .accentSoft       = argb(0x2E8A74FF), // 18%
            .onAccent         = rgb(0xFFFFFF),
            .danger           = rgb(0xFF5F72),
            .dangerText       = rgb(0xFF8191),
            .warning          = rgb(0xF5B04C),
            .warningText      = rgb(0xF7C06F),
            .success          = rgb(0x5CCF8F),
            .scrollThumb      = argb(0x33F0F0FF), // 20%
            .scrollThumbHover = argb(0x4DF0F0FF), // 30%
            .scrim            = argb(0x73000000), // 45%
            .switchOff        = rgb(0x46464C),
        },
        .elevation = {
            // Menus: 0 14px 34px rgb(0 0 0 / 45%); their strong hairline is the ring
            .overlay        = {.color = argb(0x73000000), .blur = 34, .offsetY = 14},
            .thumb          = {.color = argb(0x66000000), .blur = 2, .offsetY = 1},
            // Panels: 0 8px 24px rgb(0 0 0 / 35%) over a 1 px ring of light at 7%
            .panel          = {.color = argb(0x59000000), .blur = 24, .offsetY = 8},
            .panelContact   = {.color = argb(0x12FFFFFF), .spread = 1},
        },
        .type = floatTypography,
        .metrics = floatMetrics,
    };
}
