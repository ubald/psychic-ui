#pragma once

#include "psychic-ui/components/Range.hpp"
#include "psychic-ui/components/Label.hpp"
#include "psychic-ui/skins/paint/Box.hpp"
#include "SkinState.hpp"

namespace psychic_ui {
    /**
     * What every range skin shares, its parts: the `.track`, with the `.fill` inside it from the start up to the
     * value and the `.thumb` centered on the value, then the `.value` label; and the `.horizontal` or `.vertical`
     * class from the `orientation` style. A horizontal fill grows from the left, a vertical one from the bottom.
     * The base places the fill and the thumb from the value; the skin set's rules size and paint the parts
     * (`Range .track { height, backgroundColor }`) and lay the skin out. A press picks the value under it along
     * the track.
     */
    class RangeSkinBase : public RangeSkin {
    public:
        RangeSkinBase();

        void setValue(float value) override;

        /**
         * Along the track: its start is 0, its end 1
         */
        float valueAt(int x, int y) const override;

    protected:
        float  _value{0.0f};
        Div   *_track{nullptr};      // The `.track` part, a child, which we own
        Div   *_fill{nullptr};       // The `.fill` part, in the track
        Div   *_thumbAnchor{nullptr}; // A point on the track at the value, centering the thumb on it
        Div   *_thumb{nullptr};      // The `.thumb` part
        Label *_valueLabel{nullptr}; // The `.value` part, a child, which we own

        void added() override;
        void styleUpdated() override;
        void draw(SkCanvas *canvas) final;

        /**
         * The value changed (`_value` is the new one): the base has moved the fill and the thumb
         */
        virtual void valueChanged() {}

        /**
         * Paint the range under its parts. The default is the Div's own painting.
         */
        virtual void drawChrome(SkCanvas *canvas, const paint::Box &box, SkinState state);

    private:
        bool _vertical{false};

        void placeParts();
    };
}
