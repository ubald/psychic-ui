#include <algorithm>
#include <cmath>
#include "include/core/SkPaint.h"
#include "psychic-ui/icons/IconRegistry.hpp"
#include "Icon.hpp"

namespace psychic_ui::paint {

    bool icon(SkCanvas *canvas, const std::string_view name, const SkRect &rect, const Color color,
              const float strokeWidth, IconRegistry *registry) {
        const IconShape *shape = (registry ? *registry : IconRegistry::shared()).shape(name);
        const float     size   = std::min(rect.width(), rect.height());
        if (!shape || size <= 0.0f) {
            return shape != nullptr;
        }

        SkPoint origin{rect.centerX() - (size / 2.0f), rect.centerY() - (size / 2.0f)};
        // On the pixel grid, so the same icon looks the same wherever it is
        const SkMatrix matrix = canvas->getTotalMatrix();
        SkMatrix       inverse;
        if (matrix.isScaleTranslate() && matrix.invert(&inverse)) {
            const SkPoint device = matrix.mapPoint(origin);
            origin = inverse.mapPoint({std::round(device.x()), std::round(device.y())});
        }

        const float scale = size / shape->grid;
        SkPaint     paint;
        paint.setAntiAlias(true);
        paint.setColor(color.argb());
        paint.setStrokeCap(SkPaint::kRound_Cap);
        paint.setStrokeJoin(SkPaint::kRound_Join);
        // In grid units: the canvas is scaled by `scale`
        paint.setStrokeWidth(strokeWidth * shape->grid / 16.0f);

        canvas->save();
        canvas->translate(origin.x(), origin.y());
        canvas->scale(scale, scale);
        for (const auto &[kind, path]: shape->paths) {
            paint.setStyle(kind == IconPaint::Fill ? SkPaint::kFill_Style : SkPaint::kStroke_Style);
            canvas->drawPath(path, paint);
        }
        canvas->restore();
        return true;
    }
}
