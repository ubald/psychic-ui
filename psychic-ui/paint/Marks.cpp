#include "include/core/SkPaint.h"
#include "include/core/SkPathBuilder.h"
#include "Marks.hpp"

namespace psychic_ui::paint {

    void arrow(SkCanvas *canvas, const SkRect &bounds, const Color color, const bool antiAlias) {
        SkPaint paint;
        paint.setAntiAlias(antiAlias);
        paint.setStyle(SkPaint::kFill_Style);
        paint.setColor(color.argb());
        const SkPath path = SkPathBuilder()
            .moveTo(bounds.left(), bounds.top())
            .lineTo(bounds.right(), bounds.centerY())
            .lineTo(bounds.left(), bounds.bottom())
            .close()
            .detach();
        canvas->drawPath(path, paint);
    }

    void disc(SkCanvas *canvas, const SkPoint center, const float radius, const Color color, const bool antiAlias) {
        SkPaint paint;
        paint.setAntiAlias(antiAlias);
        paint.setStyle(SkPaint::kFill_Style);
        paint.setColor(color.argb());
        canvas->drawCircle(center, radius, paint);
    }

    void circle(SkCanvas *canvas, const SkPoint center, const float radius, const Color color, const float width, const bool antiAlias) {
        SkPaint paint;
        paint.setAntiAlias(antiAlias);
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setStrokeWidth(width);
        paint.setColor(color.argb());
        canvas->drawCircle(center, radius, paint);
    }
}
