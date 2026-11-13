#pragma once

#include <string_view>
#include "include/core/SkCanvas.h"
#include "include/core/SkRect.h"
#include "psychic-ui/style/Values.hpp"

namespace psychic_ui {
    class IconRegistry;
}

namespace psychic_ui::paint {

    /**
     * The default `iconStrokeWidth`: Pivotal's stroke, 1.3 units of a 16-unit grid
     */
    inline constexpr float iconStroke = 1.3f;

    /**
     * Draws a registered icon (IconRegistry::shared(), or `registry`) in `color`: its grid fills the largest square
     * centered in `rect`, whose corner is put on a device pixel when the canvas only scales and translates.
     * `strokeWidth` is in units of a 16-unit grid, whatever the icon's own grid, so every icon has the same weight
     * at the same size: at 16 px a width of 1.3 is 1.3 px, at 20 px 1.625 px.
     * Returns false, drawing nothing, for an unknown name (reported once).
     */
    bool icon(SkCanvas *canvas, std::string_view name, const SkRect &rect, Color color,
              float strokeWidth = iconStroke, IconRegistry *registry = nullptr);
}
