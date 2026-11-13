#pragma once

#include <SkCanvas.h>
#include "../Div.hpp"

namespace psychic_ui {

    namespace default_skin {

        static constexpr float padding = 2.0f;

        /**
         * Draws the default component chrome
         *
         * @param div - Component to draw for
         * @param canvas - Canvas to draw into
         * @return Inner SkRRect, for the caller to use to continue drawing inside the component
         */
        inline static SkRRect draw(Div *div, SkCanvas *canvas) {
            const Style &style = div->computedStyle();
            float radius = style.borderRadius();
            float strokeWidth = style.border();

            // Setup main rect
            SkRRect rect = SkRRect::MakeRectXY(
                {
                    0.0f,
                    0.0f,
                    static_cast<float>(div->getWidth()),
                    static_cast<float>(div->getHeight())
                },
                radius, radius
            );

            // Setup paint
            SkPaint paint{};
            paint.setAntiAlias(style.antiAlias());
            paint.setStyle(SkPaint::kFill_Style);
            paint.setColor(style.contentBackgroundColor().argb());

            // Paint background
            SkRRect inside = rect;
            if(strokeWidth > 0) {
                inside.inset(1.0f, 1.0f);
            }
            canvas->drawRRect(inside, paint);

            // Paint border
            if(strokeWidth > 0) {
                float strokeInset = strokeWidth > 1.0f ? 1.0f : 0.5f;
                SkRRect stroke = rect;
                stroke.inset(strokeInset, strokeInset);
                paint.setStyle(SkPaint::kStroke_Style);
                paint.setStrokeWidth(strokeWidth);
                paint.setColor(style.borderColor().argb());
                canvas->drawRRect(stroke, paint);
            }

            rect.inset(2.0f, 2.0f);

            return rect;
        }
    }

}
