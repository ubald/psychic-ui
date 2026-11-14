#pragma once

#include <vector>
#include "psychic-ui/components/SegmentedControl.hpp"
#include "psychic-ui/skins/paint/Box.hpp"
#include "SkinState.hpp"

namespace psychic_ui {
    /**
     * What every segmented control skin shares, its parts: a `.segment` per segment, in a row, holding its `.icon`
     * (an Icon) and its `.label`, whichever it has; the selected one has the `.selected` class
     * (`SegmentedControl .segment.selected`). The skin set's rules size and paint them.
     */
    class SegmentedControlSkinBase : public SegmentedControlSkin {
    public:
        SegmentedControlSkinBase();

        void setSegments(const std::vector<Segment> &segments) override;
        void setSelected(int index) override;

        /**
         * The segment under the point, else the nearest along the row
         */
        int segmentAt(int x, int y) const override;

    protected:
        std::vector<Div *> _segments{}; // The `.segment` parts, children, which we own
        int                _selected{-1};

        void draw(SkCanvas *canvas) final;

        /**
         * Paint the control under its segments. The default is the Div's own painting.
         */
        virtual void drawChrome(SkCanvas *canvas, const paint::Box &box, SkinState state);
    };
}
