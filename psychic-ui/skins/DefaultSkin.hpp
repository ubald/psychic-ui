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
            float radius = div->computedStyle()->get(borderRadius);

            SkRRect rect = SkRRect::MakeRectXY(
                {
                    0.0f,
                    0.0f,
                    static_cast<float>(div->getWidth()),
                    static_cast<float>(div->getHeight())
                },
                radius, radius
            );

            SkPaint paint{};
            paint.setAntiAlias(div->computedStyle()->get(antiAlias));
            paint.setStyle(SkPaint::kFill_Style);
            paint.setColor(div->computedStyle()->get(contentBackgroundColor));
            if (div->computedStyle()->has(opacity)) {
                paint.setAlpha((unsigned int) (div->computedStyle()->get(opacity) * 255.f));
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

}
