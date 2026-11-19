#pragma once

#include "include/core/SkCanvas.h"
#include "include/core/SkRect.h"
#include "psychic-ui/components/Button.hpp"
#include "psychic-ui/style/Color.hpp"

namespace psychic_ui {
    /**
     * A window control as Windows and Linux draw them (the "caption" variant): the button's background in its
     * rounded border box, and a glyph in its `color` from its class: a line for `minimize`, a square for `maximize`
     * (two offset squares, restore, while the window is maximized), a cross for `close`. The glyph is `iconSize`
     * wide, stroked at `iconStrokeWidth` (in units of a 16-unit grid, as icons are) on whole device pixels.
     * It has no label, so it skips ButtonSkinBase.
     */
    class CaptionButtonSkin : public ButtonSkin {
    public:
        CaptionButtonSkin();

    protected:
        void draw(SkCanvas *canvas) override;

    private:
        static void drawMinimize(SkCanvas *canvas, const SkRect &glyph, float stroke, Color color);
        static void drawMaximize(SkCanvas *canvas, const SkRect &glyph, float stroke, Color color);
        static void drawRestore(SkCanvas *canvas, const SkRect &glyph, float stroke, Color color);
        static void drawClose(SkCanvas *canvas, const SkRect &glyph, float stroke, Color color);
    };
}
