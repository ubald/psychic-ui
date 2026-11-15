#pragma once

#include "Theme.hpp"

/**
 * Pivotal's design system (packages/ui/src/styles/tokens.css), translated value for value: P1 Faithful, the default
 * skin set's look. Translucent tokens keep their alpha; Pivotal's `color-mix` values are precomputed.
 * Typography and metrics are the defaults of their structs, which are Pivotal's.
 */
namespace psychic_ui::themes {
    /**
     * Pivotal's dark theme, its default: a near-black base with barely-lighter raised surfaces, hairline borders and
     * one indigo accent
     */
    inline constexpr Theme pivotalDark{
        .name = "Pivotal Dark",
        .scheme = ColorScheme::Dark,
        .palette = {
            .base             = rgb(0x08090A),
            .surface          = rgb(0x0E0F11),
            .raised           = rgb(0x151619),
            .overlay          = rgb(0x0C0D0F),
            .frame            = rgb(0x0E0F11), // --color-surface
            .page             = rgb(0x08090A), // --color-base
            .hover            = argb(0x99151619), // color-mix(in srgb, raised 60%, transparent)
            .line             = argb(0x12FFFFFF), // rgb(255 255 255 / 7%)
            .lineStrong       = argb(0x1FFFFFFF), // 12%
            .text             = rgb(0xF7F8F8),
            .textSecondary    = rgb(0x9BA1A6),
            .textMuted        = rgb(0x7A8086),
            .accent           = rgb(0x5E6AD2),
            .accentHover      = rgb(0x6E79D6),
            .accentText       = rgb(0x6E79D6),
            .accentSoft       = argb(0x245E6AD2), // 14%
            .onAccent         = rgb(0xFFFFFF),
            .danger           = rgb(0xE5484D),
            .dangerText       = rgb(0xEF6B6F),
            .warning          = rgb(0xF2994A),
            .warningText      = rgb(0xF2994A),
            .success          = rgb(0x4CB782),
            .scrollThumb      = argb(0x17FFFFFF), // 9%
            .scrollThumbHover = argb(0x29FFFFFF), // 16%
            .scrim            = argb(0x80000000), // 50%
            .switchOff        = rgb(0x151619), // --color-raised
            .canvasDot        = argb(0x1FFFFFFF), // The strong hairline
        },
        .elevation = {
            // --shadow-overlay: 0 12px 32px -8px rgb(0 0 0 / 55%), 0 2px 6px rgb(0 0 0 / 30%)
            .overlay        = {.color = argb(0x8C000000), .blur = 32, .offsetY = 12, .spread = -8},
            .overlayContact = {.color = argb(0x4D000000), .blur = 6, .offsetY = 2},
            // --shadow-thumb: 0 1px 2px rgb(0 0 0 / 30%)
            .thumb          = {.color = argb(0x4D000000), .blur = 2, .offsetY = 1},
            // Panels over a canvas, lower than overlays: 0 8px 24px -6px rgb(0 0 0 / 45%), ringed in white at 18%
            // (past the strong hairline, which a dark shadow swallows), since the canvas (the frame) is their color
            .panel          = {.color = argb(0x73000000), .blur = 24, .offsetY = 8, .spread = -6},
            .panelContact   = {.color = argb(0x2EFFFFFF), .spread = 1},
        },
    };

    /**
     * Pivotal's light theme: the same restraint on a near-white base; layers separate by a step of grey, overlays
     * float white above everything
     */
    inline constexpr Theme pivotalLight{
        .name = "Pivotal Light",
        .scheme = ColorScheme::Light,
        .palette = {
            .base             = rgb(0xFBFBFC),
            .surface          = rgb(0xF4F5F6),
            .raised           = rgb(0xECEEF0),
            .overlay          = rgb(0xFFFFFF),
            .frame            = rgb(0xF4F5F6),
            .page             = rgb(0xFBFBFC),
            .hover            = argb(0x99ECEEF0),
            .line             = argb(0x14000000), // rgb(0 0 0 / 8%)
            .lineStrong       = argb(0x24000000), // 14%
            .text             = rgb(0x1B1C1F),
            .textSecondary    = rgb(0x43484D),
            .textMuted        = rgb(0x646A70),
            .accent           = rgb(0x5E6AD2),
            .accentHover      = rgb(0x4E59C2),
            .accentText       = rgb(0x4E59C2),
            .accentSoft       = argb(0x1F5E6AD2), // 12%
            .onAccent         = rgb(0xFFFFFF),
            .danger           = rgb(0xCD2B31),
            .dangerText       = rgb(0xB22328),
            .warning          = rgb(0xB25A0E),
            .warningText      = rgb(0x944B09),
            .success          = rgb(0x1A7A4C),
            .scrollThumb      = argb(0x24000000), // 14%
            .scrollThumbHover = argb(0x3D000000), // 24%
            .scrim            = argb(0x3D000000), // 24%
            .switchOff        = rgb(0xC4C8CD),
            .canvasDot        = argb(0x24000000), // The strong hairline
        },
        .elevation = {
            .overlay        = {.color = argb(0x29000000), .blur = 32, .offsetY = 12, .spread = -8},
            .overlayContact = {.color = argb(0x14000000), .blur = 6, .offsetY = 2},
            .thumb          = {.color = argb(0x33000000), .blur = 2, .offsetY = 1},
            .panel          = {.color = argb(0x1F000000), .blur = 24, .offsetY = 8, .spread = -6},
            .panelContact   = {.color = argb(0x24000000), .spread = 1}, // The strong hairline
        },
    };
}
