#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRRect.h"
#include "Box.hpp"
#include "psychic-ui/Div.hpp"
#include "psychic-ui/style/Color.hpp"
#include "psychic-ui/style/Style.hpp"

namespace psychic_ui::paint {

    Box Box::of(const Div &div) {
        const Style &style  = div.computedStyle();
        const float radius = style.borderRadius();
        return Box{
            .outer = SkRRect::MakeRectXY(
                SkRect::MakeWH(static_cast<float>(div.getWidth()), static_cast<float>(div.getHeight())),
                radius, radius
            ),
            .border = style.border()
        };
    }

    SkRRect Box::inset(const float amount) const {
        SkRRect rect = outer;
        rect.inset(amount, amount);
        return rect;
    }

    void fill(SkCanvas *canvas, const SkRRect &shape, const Color color, const bool antiAlias) {
        SkPaint paint{};
        paint.setAntiAlias(antiAlias);
        paint.setStyle(SkPaint::kFill_Style);
        paint.setColor(color.argb());
        canvas->drawRRect(shape, paint);
    }

    void fill(SkCanvas *canvas, const SkRect &shape, const Color color, const bool antiAlias) {
        SkPaint paint{};
        paint.setAntiAlias(antiAlias);
        paint.setStyle(SkPaint::kFill_Style);
        paint.setColor(color.argb());
        canvas->drawRect(shape, paint);
    }

    void stroke(SkCanvas *canvas, const SkRRect &shape, const Color color, const float width, const bool antiAlias) {
        SkPaint paint{};
        paint.setAntiAlias(antiAlias);
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setStrokeWidth(width);
        paint.setColor(color.argb());
        canvas->drawRRect(shape, paint);
    }
}
