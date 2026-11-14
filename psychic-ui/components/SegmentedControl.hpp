#pragma once

#include <string>
#include <vector>
#include "psychic-ui/psychic-ui.hpp"
#include "psychic-ui/Component.hpp"
#include "psychic-ui/Skin.hpp"
#include "psychic-ui/signals/Signal.hpp"

namespace psychic_ui {
    class SegmentedControl;

    /**
     * One choice of a segmented control: a label, an icon (a registered icon's name), or both
     */
    struct Segment {
        std::string label{};
        std::string icon{};
    };

    class SegmentedControlSkin : public Skin<SegmentedControl> {
    public:
        SegmentedControlSkin() : Skin<SegmentedControl>() {
            tag("SegmentedControlSkin");
        }

        virtual void setSegments(const std::vector<Segment> &/*segments*/) {};
        virtual void setSelected(int /*index*/) {};

        /**
         * The segment at this point, in the skin's coordinates, or the nearest one; -1 when there are none.
         * The control handles the mouse; the skin only knows where its segments are.
         */
        virtual int segmentAt(int x, int y) const = 0;
    };

    /**
     * @class SegmentedControl
     *
     * One of a few mutually exclusive choices, all visible ("Day / Week / Month"), like a radio group: one segment
     * is selected (the first by default); a click picks one, and with the focus Left and Right (or Up and Down)
     * move the selection, Home and End go to the ends.
     *
     *     add<SegmentedControl>(std::vector<Segment>{{"Day"}, {"Week"}, {"Month"}}).selected(1);
     */
    class SegmentedControl : public Component<SegmentedControlSkin> {
    public:
        explicit SegmentedControl(std::vector<Segment> segments = {});

        const std::vector<Segment> &segments() const;

        /**
         * Replace the segments, keeping the selected index if it still exists (else the first)
         */
        template<class Self>
        Self &segments(this Self &self, std::vector<Segment> segments) {
            static_cast<SegmentedControl &>(self).updateSegments(std::move(segments));
            return self;
        }

        /**
         * The selected segment's index, -1 when there are no segments
         */
        int selected() const;

        /**
         * Select a segment by its index, clamped to the segments
         */
        template<class Self>
        Self &selected(this Self &self, int index) {
            static_cast<SegmentedControl &>(self).updateSelected(index);
            return self;
        }

        /**
         * The selection changed, with the new index
         */
        Signal<int> changed{};
        PSYCHIC_UI_EVENT(onChange, changed)

    protected:
        std::vector<Segment> _segments{};
        int                  _selected{-1};
        int                  _pressed{-1}; // The segment the left button went down on

        void updateSegments(std::vector<Segment> segments);
        void updateSelected(int index);
        void skinChanged() override;
    };
}
