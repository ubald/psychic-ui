#pragma once

#include "psychic-ui/components/Range.hpp"
#include "psychic-ui/components/Label.hpp"
#include "psychic-ui/skins/paint/Box.hpp"
#include "SkinState.hpp"

namespace psychic_ui {
    /**
     * What every range skin shares: the `.value` part, a label showing the value, centered over the skin, and
     * the `.horizontal` or `.vertical` class from the `orientation` style.
     * A skin set's range paints its track and fill, from `_value` (0 to 1, linear).
     */
    class RangeSkinBase : public RangeSkin {
    public:
        RangeSkinBase();

        void setValue(float value) override;

    protected:
        float  _value{0.0f};
        Label *_valueLabel{nullptr}; // The `.value` part, a grandchild, which we own

        void added() override;
        void styleUpdated() override;
        void draw(SkCanvas *canvas) final;

        /**
         * The value changed (`_value` is the new one)
         */
        virtual void valueChanged() {}

        /**
         * Paint the range under its value label. The default is the Div's own painting.
         */
        virtual void drawChrome(SkCanvas *canvas, const paint::Box &box, SkinState state);
    };
}
