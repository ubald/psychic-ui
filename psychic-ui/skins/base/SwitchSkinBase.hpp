#pragma once

#include "psychic-ui/components/Switch.hpp"
#include "psychic-ui/components/Label.hpp"
#include "psychic-ui/paint/Box.hpp"
#include "SkinState.hpp"

namespace psychic_ui {
    /**
     * What every switch skin shares, its parts: the `.track`, with the `.thumb` in it, then the `.label`.
     * The thumb travels the track's content box, from its start (off) to its end (on): the base places it from the
     * switch's thumb position; the skin set's rules size and paint the parts (`Switch .track { width, padding }`,
     * `Switch.checked .track { backgroundColor }`).
     */
    class SwitchSkinBase : public SwitchSkin {
    public:
        SwitchSkinBase();

        void setLabel(const std::string &label) override;
        void setThumbPosition(float position) override;

        /**
         * Along the thumb's travel: its center at the start of the track's content is 0, at the end 1
         */
        float positionAt(int x, int y) const override;

    protected:
        Div   *_track{nullptr};  // The `.track` part, a child: our child list owns it
        Div   *_before{nullptr}; // Grows by the position, before the thumb
        Div   *_thumb{nullptr};  // The `.thumb` part, in the track
        Div   *_after{nullptr};  // Grows by what's left, after the thumb
        Label *_label{nullptr};  // The `.label` part, a child: our child list owns it

        void draw(SkCanvas *canvas) final;

        /**
         * Paint the switch under its parts. The default is the Div's own painting.
         */
        virtual void drawChrome(SkCanvas *canvas, const paint::Box &box, SkinState state);
    };
}
