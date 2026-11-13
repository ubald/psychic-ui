#pragma once

#include "psychic-ui/skins/base/RangeSkinBase.hpp"

namespace psychic_ui {
    /**
     * The default range: the default chrome as the track, filled from the start up to the value. The skin gets the
     * `.inverted` class once the fill reaches its middle, under the centered value label (`.normal` before).
     */
    class DefaultSliderSkin : public RangeSkinBase {
    public:
        DefaultSliderSkin();

        float valueAt(int x, int y) const override;

    protected:
        void valueChanged() override;
        void drawChrome(SkCanvas *canvas, const paint::Box &box, SkinState state) override;
    };
}
