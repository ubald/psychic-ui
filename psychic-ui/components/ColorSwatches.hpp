#pragma once

#include <vector>
#include "psychic-ui/psychic-ui.hpp"
#include "psychic-ui/Component.hpp"
#include "psychic-ui/Skin.hpp"
#include "psychic-ui/signals/Signal.hpp"
#include "psychic-ui/style/Values.hpp"

namespace psychic_ui {
    class ColorSwatches;

    class ColorSwatchesSkin : public Skin<ColorSwatches> {
    public:
        ColorSwatchesSkin() : Skin<ColorSwatches>() {
            tag("ColorSwatchesSkin");
        }

        virtual void setColors(const std::vector<Color> &/*colors*/) {};
        virtual void setSelected(int /*index*/) {};

        /**
         * The swatch at this point, in the skin's coordinates, or the nearest one; -1 when there are none.
         * The swatches handle the mouse; the skin only knows where each one is.
         */
        virtual int swatchAt(int x, int y) const = 0;
    };

    /**
     * @class ColorSwatches
     *
     * A color picked from a palette of chips, wrapping into rows like a grid: the selected one is ringed. A click
     * picks one; with the focus, the arrows move the selection to the previous or next chip (Left and Up, Right and
     * Down, like a radio group), Home and End to the ends. Pivotal's palette by default.
     */
    class ColorSwatches : public Component<ColorSwatchesSkin> {
    public:
        /**
         * Pivotal's palette (`palette::pivotalSwatches`): eighteen hues, then a brown and a grey
         */
        static const std::vector<Color> &pivotalPalette();

        explicit ColorSwatches(std::vector<Color> colors = pivotalPalette());

        const std::vector<Color> &colors() const;

        /**
         * Replace the colors, keeping the selected color if it is still one of them (else none)
         */
        template<class Self>
        Self &colors(this Self &self, std::vector<Color> colors) {
            static_cast<ColorSwatches &>(self).updateColors(std::move(colors));
            return self;
        }

        /**
         * The selected chip's index, -1 for none (the default)
         */
        int selected() const;

        /**
         * Select a chip by its index; -1 (or another index out of the palette) selects none
         */
        template<class Self>
        Self &selected(this Self &self, int index) {
            static_cast<ColorSwatches &>(self).updateSelected(index);
            return self;
        }

        /**
         * The selected color, transparent when none is
         */
        Color value() const;

        /**
         * Select the first chip of this color, or none if the palette lacks it
         */
        template<class Self>
        Self &value(this Self &self, Color color) {
            static_cast<ColorSwatches &>(self).updateValue(color);
            return self;
        }

        /**
         * The selection changed, with the new color (transparent for none)
         */
        Signal<Color> changed{};
        PSYCHIC_UI_EVENT(onChange, changed)

    protected:
        std::vector<Color> _colors{};
        int                _selected{-1};
        int                _pressed{-1}; // The chip the left button went down on

        void updateColors(std::vector<Color> colors);
        void updateSelected(int index);
        void updateValue(Color color);
        void skinChanged() override;
    };
}
