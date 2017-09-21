#pragma once

#include <SkCanvas.h>
#include "../Div.hpp"

namespace psychic_ui {

    /**
     * Draws the default component chrome
     *
     * @param div - Component to draw for
     * @param canvas - Canvas to draw into
     * @return Inner SkRRect, for the caller to use to continue drawing inside the component
     */
    static inline SkRRect drawDefaultSkinChrome(float width, float height, const Style *style, SkCanvas *canvas) {
        float radius = style->get(borderRadius);

        SkRRect rect = SkRRect::MakeRectXY(
            {0, 0, width, height},
            radius, radius
        );

        SkPaint paint{};
        paint.setAntiAlias(style->get(antiAlias));
        paint.setStyle(SkPaint::kFill_Style);
        paint.setColor(style->get(contentBackgroundColor));
        if (style->has(opacity)) {
            paint.setAlpha((unsigned int) (style->get(opacity) * 255.f));
        }

        SkRRect inside = rect;
        inside.inset(1.0f, 1.0f);
        canvas->drawRRect(inside, paint);

        //SkRRect stroke = rect;
        //stroke.inset(0.5f, 0.5f);
        //inside.inset(1.0f, 1.0f);
        //paint.setStyle(SkPaint::kStroke_Style);
        //paint.setColor(style->get(borderColor));
        //canvas->drawRRect(stroke, paint);

        rect.inset(2.0f, 2.0f);

        return rect;
    }

}
