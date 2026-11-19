#pragma once

#include "psychic-ui/Component.hpp"
#include "psychic-ui/Skin.hpp"
#include "psychic-ui/style/Values.hpp"

namespace psychic_ui {
    class SplitterDivider;

    /**
     * What a splitter's divider is drawn by. It takes no data: the divider's orientation reaches the skin through the
     * `orientation` style it inherits, and its states through the style (`:hover`, `:pressed`, `:focus-visible`).
     */
    class SplitterDividerSkin : public Skin<SplitterDivider> {
    public:
        SplitterDividerSkin() : Skin<SplitterDivider>() {
            tag("SplitterDividerSkin");
        }
    };

    /**
     * The grip between a `Splitter`'s panes: an absolute strip over the boundary, wider than the line its skin draws
     * in its middle, so the mouse catches it easily. The splitter places it, sets its `orientation` (and the
     * `horizontal` or `vertical` class, as a scroll bar's) and handles its presses, drags and keys; the divider takes
     * the focus and shows the resize cursor across the boundary.
     */
    class SplitterDivider : public Component<SplitterDividerSkin> {
    public:
        SplitterDivider();

        /**
         * How wide it is across the boundary by default (window units), what a press catches; rules change it
         */
        static constexpr float defaultReach = 8;

    protected:
        Orientation _orientation{Orientation::Horizontal};
        bool        _oriented{false}; // Whether `_orientation` was applied yet

        void styleUpdated() override;
    };
}
