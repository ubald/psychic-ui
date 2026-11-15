#pragma once

#include <vector>
#include "psychic-ui/components/ColorSwatches.hpp"
#include "psychic-ui/skins/paint/Box.hpp"
#include "SkinState.hpp"

namespace psychic_ui {
    /**
     * What every color swatches skin shares, its parts: a `.swatch` per color, its background that color (inline,
     * so rules can't override it), the selected one with the `.selected` class (`ColorSwatches .swatch.selected`),
     * the one the keyboard is on with the `.focused` class (`ColorSwatches:focus-visible .swatch.focused`).
     * The skin set's rules size, shape and lay them out (wrapping them makes a grid).
     */
    class ColorSwatchesSkinBase : public ColorSwatchesSkin {
    public:
        ColorSwatchesSkinBase();

        void setColors(const std::vector<Color> &colors) override;
        void setSelected(int index) override;
        void setFocused(int index) override;

        /**
         * The swatch under the point, else the one whose center is nearest
         */
        int swatchAt(int x, int y) const override;

    protected:
        std::vector<Div *> _swatches{}; // The `.swatch` parts, children, which we own
        int                _selected{-1};
        int                _focused{-1};

        void draw(SkCanvas *canvas) final;

        /**
         * Paint under the swatches. The default is the Div's own painting.
         */
        virtual void drawChrome(SkCanvas *canvas, const paint::Box &box, SkinState state);
    };
}
