#pragma once

#include <array>
#include <cstdint>
#include "psychic-ui/Div.hpp"
#include "psychic-ui/signals/Signal.hpp"
#include "psychic-ui/signals/Slot.hpp"
#include "psychic-ui/style/Values.hpp"

namespace psychic_ui {
    class SplitterDivider;

    /**
     * One of a splitter's two panes: the first is at the start (left, or top), the second at the end
     */
    enum class SplitterPane : std::uint8_t { First, Second };

    /**
     * Two panes side by side (the `orientation` style, horizontal by default) or stacked (vertical), and a divider
     * between them that the mouse, a finger or the arrows move.
     *
     * One pane is kept: it keeps its size, the position, when the splitter resizes, and the other takes the rest (a
     * docked inspector is the second pane, kept). The position is the kept pane's size along the splitter, within
     * each pane's minimum; a position that doesn't fit is laid out as near as fits, and comes back when there is room.
     * A double-click on the divider goes back to the default position.
     *
     *     auto &split = add<Splitter>().kept(SplitterPane::Second).defaultPosition(280)
     *                                  .minimum(SplitterPane::First, 320).minimum(SplitterPane::Second, 200);
     *     split.first().add<Canvas>();
     *     split.second().add<Inspector>();
     *
     * The panes are the splitter's first two children, the `SplitterDivider` (a component, which the skin set draws)
     * its last, laid over the boundary so it catches presses on either side of the line it shows.
     */
    class Splitter : public Div {
    public:
        Splitter();

        /**
         * The first pane, at the start: add its content to it
         */
        Div &first() const { return *_first; }

        /**
         * The second pane, at the end
         */
        Div &second() const { return *_second; }

        /**
         * A pane, by name
         */
        Div &pane(SplitterPane pane) const { return pane == SplitterPane::First ? *_first : *_second; }

        /**
         * The divider between the panes
         */
        SplitterDivider &divider() const { return *_divider; }

        /**
         * The pane that keeps its size when the splitter resizes (the first by default)
         */
        SplitterPane kept() const { return _kept; }

        template<class Self>
        Self &kept(this Self &self, SplitterPane pane) {
            static_cast<Splitter &>(self).updateKept(pane);
            return self;
        }

        /**
         * The kept pane's size along the splitter (window units), within the minimums once laid out
         */
        float position() const;

        /**
         * Set the position, without emitting `changed`
         */
        template<class Self>
        Self &position(this Self &self, float position) {
            static_cast<Splitter &>(self).updatePosition(position);
            return self;
        }

        /**
         * Where a double-click on the divider puts it back (240 by default)
         */
        float defaultPosition() const { return _defaultPosition; }

        /**
         * Set the default position, and the position with it
         */
        template<class Self>
        Self &defaultPosition(this Self &self, float position) {
            auto &splitter = static_cast<Splitter &>(self);
            splitter._defaultPosition = position;
            splitter.updatePosition(position);
            return self;
        }

        /**
         * The smallest a pane gets from the divider (0 by default)
         */
        float minimum(SplitterPane pane) const { return _minimums.at(index(pane)); }

        template<class Self>
        Self &minimum(this Self &self, SplitterPane pane, float minimum) {
            static_cast<Splitter &>(self).updateMinimum(pane, minimum);
            return self;
        }

        /**
         * How far an arrow moves the divider (window units); ten times that with Shift
         */
        static constexpr float keyStep = 10;

        /**
         * The divider moved, from a drag, a key or a double-click, with the new position
         */
        Signal<float> changed{};
        PSYCHIC_UI_EVENT(onChange, changed)

        void layoutUpdated() override;

    protected:
        Div             *_first{nullptr};   // Our children: our child list owns them
        Div             *_second{nullptr};
        SplitterDivider *_divider{nullptr};

        SplitterPane         _kept{SplitterPane::First};
        float                _position{240};
        float                _defaultPosition{240};
        std::array<float, 2> _minimums{0, 0};
        Orientation          _orientation{Orientation::Horizontal};
        bool                 _oriented{false}; // Whether `_orientation` was applied yet

        // Where the layout put the panes along the splitter, from the last layout: the first's start, the second's end
        float _start{0};
        float _end{0};
        bool  _laidOut{false};

        // The drag
        float      _pressAt{0};       // Where the divider was pressed along the splitter, in window coordinates
        float      _pressPosition{0}; // The position then
        ScopedSlot _onMouseMove{};    // On the window, only while dragging

        static constexpr std::size_t index(SplitterPane pane) { return pane == SplitterPane::First ? 0 : 1; }

        /**
         * The position within the minimums and the room the panes have
         */
        float fit(float position) const;

        /**
         * A position from the user: fitted, applied, and emitted when it changed
         */
        void moveTo(float position);

        /**
         * Push the position to the panes and the divider
         */
        void place();

        void updateKept(SplitterPane pane);
        void updatePosition(float position);
        void updateMinimum(SplitterPane pane, float minimum);
        void styleUpdated() override;
        void removedFromRender() override;
    };
}
