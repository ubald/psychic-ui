#include <algorithm>
#include <array>
#include <cmath>
#include <optional>
#include <span>
#include "include/core/SkCanvas.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRRect.h"
#include "include/core/SkRect.h"
#include "Box.hpp"
#include "Crisp.hpp"
#include "psychic-ui/style/Color.hpp"

namespace psychic_ui::paint {

    namespace {
        // A canvas whose matrix only scales, the same on both axes, and translates: the device grid in local units
        struct Grid {
            SkMatrix matrix{};
            SkMatrix inverse{};
            float    scale{1};
        };

        std::optional<Grid> gridOf(const SkCanvas *canvas) {
            Grid grid{.matrix = canvas->getTotalMatrix()};
            if (!grid.matrix.isScaleTranslate() || !grid.matrix.invert(&grid.inverse)) {
                return std::nullopt;
            }
            const float scaleX = std::abs(grid.matrix.getScaleX());
            if (scaleX != std::abs(grid.matrix.getScaleY())) {
                return std::nullopt;
            }
            grid.scale = scaleX;
            return grid;
        }

        float crispWidth(const std::optional<Grid> &grid, const float width) {
            if (!grid) {
                return width;
            }
            return std::max(1.0f, std::round(width * grid->scale)) / grid->scale;
        }

        SkRRect snap(const std::optional<Grid> &grid, const SkRRect &shape) {
            if (!grid || shape.isEmpty()) {
                return shape;
            }
            // mapRect sorts the edges, so a mirrored canvas works too
            const SkRect device = grid->matrix.mapRect(shape.rect());
            const SkRect rect   = grid->inverse.mapRect(SkRect::Make(device.round()));
            const std::array radii{
                shape.radii(SkRRect::kUpperLeft_Corner), shape.radii(SkRRect::kUpperRight_Corner),
                shape.radii(SkRRect::kLowerRight_Corner), shape.radii(SkRRect::kLowerLeft_Corner)
            };
            SkRRect snapped;
            snapped.setRectRadii(rect, radii.data());
            return snapped;
        }

        SkRRect inset(const SkRRect &shape, const float amount) {
            SkRRect rect = shape;
            rect.inset(amount, amount);
            return rect;
        }
    }

    float crispWidth(const SkCanvas *canvas, const float width) {
        return crispWidth(gridOf(canvas), width);
    }

    SkRRect snap(const SkCanvas *canvas, const SkRRect &shape) {
        return snap(gridOf(canvas), shape);
    }

    SkRRect crispInset(const SkCanvas *canvas, const SkRRect &shape, const float amount) {
        const auto grid    = gridOf(canvas);
        const auto snapped = snap(grid, shape);
        if (amount <= 0.0f) {
            return snapped;
        }
        return inset(snapped, crispWidth(grid, amount));
    }

    SkRRect rings(SkCanvas *canvas, const SkRRect &shape, const std::span<const Ring> rings,
                  const std::optional<Color> fill) {
        const auto grid    = gridOf(canvas);
        SkRRect    current = snap(grid, shape);
        for (const Ring &ring: rings) {
            if (current.isEmpty()) {
                return current;
            }
            const float   width = crispWidth(grid, ring.width);
            const SkRRect next  = inset(current, width);
            if (next.isEmpty()) {
                // Wider than what is left: the ring covers it all
                paint::fill(canvas, current, ring.color);
                return next;
            }
            // Between two rects rather than a stroke along its middle: a stroke's outline, offset both ways from a
            // line, misses the device pixels by a rounding error and bleeds faintly onto the next one
            SkPaint paint{};
            paint.setAntiAlias(true);
            paint.setColor(ring.color.argb());
            canvas->drawDRRect(current, next, paint);
            current = next;
        }
        if (fill && !current.isEmpty()) {
            paint::fill(canvas, current, *fill);
        }
        return current;
    }
}
