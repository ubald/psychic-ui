#pragma once

#include "include/core/SkCanvas.h"
#include "include/core/SkRRect.h"
#include "psychic-ui/style/Color.hpp"

namespace psychic_ui {
    class Div;
}

/**
 * Painting primitives for skins: no state, everything comes from the arguments
 */
namespace psychic_ui::paint {

    /**
     * A div's border box as a Skia rounded rect, in its own coordinates, from its size and computed radius and border
     */
    struct Box {
        SkRRect outer{};  // The border box
        float   border{0}; // The border's width

        static Box of(const Div &div);

        /**
         * The border box inset on every side (radii shrink with it)
         */
        [[nodiscard]] SkRRect inset(float amount) const;

        /**
         * Inside the border
         */
        [[nodiscard]] SkRRect inner() const { return inset(border); }

        /**
         * Where a stroke of the border's width goes to fill the border: its center line
         */
        [[nodiscard]] SkRRect strokeLine() const { return inset(border / 2.0f); }
    };

    void fill(SkCanvas *canvas, const SkRRect &shape, Color color, bool antiAlias = true);
    void fill(SkCanvas *canvas, const SkRect &shape, Color color, bool antiAlias = true);
    void stroke(SkCanvas *canvas, const SkRRect &shape, Color color, float width, bool antiAlias = true);
}
