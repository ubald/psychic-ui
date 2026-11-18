#pragma once

#include <optional>
#include <span>
#include "include/core/SkCanvas.h"
#include "include/core/SkRRect.h"
#include "psychic-ui/style/Color.hpp"

/**
 * Strokes and fills whose straight edges land on device pixels, at any pixel ratio.
 *
 * A 1 px stroke centered on a whole-pixel line covers two device rows at half intensity and looks blurry; these
 * helpers round widths to whole device pixels and snap shapes to the device grid, so only the corners' curves are
 * anti-aliased. They snap only when the canvas's total matrix is a uniform scale and a translation (a window's
 * pixel ratio and the divs' offsets); under any other transform (a popup scaling in, a rotation, uneven scales) they
 * draw exactly what they were given, as `paint::stroke` and `paint::fill` would.
 */
namespace psychic_ui::paint {

    /**
     * One stroke of a series drawn from the outside in, its width in window units
     */
    struct Ring {
        Color color{};
        float width{1};
    };

    /**
     * `width` in window units rounded to whole device pixels, at least one, in the canvas's local units; `width`
     * itself when the canvas isn't snapped
     */
    [[nodiscard]] float crispWidth(const SkCanvas *canvas, float width);

    /**
     * The shape with each edge of its rect on the nearest device pixel, its radii kept; the shape itself when the
     * canvas isn't snapped or the shape is empty
     */
    [[nodiscard]] SkRRect snap(const SkCanvas *canvas, const SkRRect &shape);

    /**
     * The snapped shape inset on every side by `crispWidth(amount)` (its radii shrink, as `SkRRect::inset` does);
     * an `amount` of 0 or less is the snapped shape
     */
    [[nodiscard]] SkRRect crispInset(const SkCanvas *canvas, const SkRRect &shape, float amount);

    /**
     * Paints nested rings from the snapped shape inward, each its crisp width inside the previous one so that they
     * touch without gaps or overlaps, then fills what is left inside them with `fill`, if any. Returns what is left:
     * the inside of the last ring, empty once the rings used up the shape. A ring wider than what is left fills it,
     * and the rings after it and the fill are skipped.
     */
    SkRRect rings(
        SkCanvas *canvas, const SkRRect &shape, std::span<const Ring> rings, std::optional<Color> fill = std::nullopt
    );
}
