#pragma once

#include "psychic-ui/signals/Slot.hpp"
#include "psychic-ui/skins/base/RangeSkinBase.hpp"

namespace psychic_ui {
    /**
     * The default range: the default chrome as the track, filled from the start up to the value. The skin gets the
     * `.inverted` class once the fill reaches its middle, under the centered value label (`.normal` before).
     */
    class DefaultSliderSkin : public RangeSkinBase {
    public:
        DefaultSliderSkin();

    protected:
        ScopedSlot _onMouseMove{}; // On the window, only while dragging

        void valueChanged() override;
        void removedFromRender() override;
        void sendMouseValue(int x, int y);
        void drawChrome(SkCanvas *canvas, const paint::Box &box, SkinState state) override;
    };
}
