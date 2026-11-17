#pragma once

#include "include/core/SkCanvas.h"
#include "psychic-ui/style/Values.hpp"

/**
 * Small shapes skins draw: arrows, circles
 */
namespace psychic_ui::paint {

    /**
     * A filled triangle pointing right, filling its bounds (a submenu's arrow)
     */
    void arrow(SkCanvas *canvas, const SkRect &bounds, Color color, bool antiAlias = true);

    /**
     * A filled circle
     */
    void disc(SkCanvas *canvas, SkPoint center, float radius, Color color, bool antiAlias = true);

    /**
     * A circle's outline; a width of 0 is a hairline, one pixel whatever the scale
     */
    void circle(SkCanvas *canvas, SkPoint center, float radius, Color color, float width = 0.0f, bool antiAlias = true);
}
