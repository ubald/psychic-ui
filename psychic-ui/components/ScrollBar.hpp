#pragma once

#include "include/core/SkRect.h"
#include "psychic-ui/Component.hpp"
#include "psychic-ui/signals/Slot.hpp"

namespace psychic_ui {
    class ScrollBar;

    class ScrollBarSkin : public Skin<ScrollBar> {
    public:
        ScrollBarSkin() : Skin<ScrollBar>() {
            tag("ScrollBarSkin");
        }

        virtual void updateScrollBar(bool enabled, float scrollPosition, float contentRatio) = 0;

        /**
         * Where the thumb moves, in the skin's coordinates
         */
        virtual SkRect trackRect() const = 0;

        /**
         * Where the thumb is, in the skin's coordinates. The scroll bar handles the mouse: a press on the thumb
         * drags it along the track.
         */
        virtual SkRect thumbRect() const = 0;
    };

    using ScrollDirection = Orientation;

    class ScrollBar : public Component<ScrollBarSkin> {
    public:
        /**
         * A scroll bar scrolling a viewport, which it keeps alive
         */
        explicit ScrollBar(Div &viewport, ScrollDirection direction = ScrollDirection::Vertical);
        Div *viewport() const;
        ScrollDirection scrollDirection() const;
        void scrollPercentX(float scrollPercentX);
        void scrollPercentY(float scrollPercentY);
    protected:
        ScrollDirection      _direction{ScrollDirection::Vertical};
        std::shared_ptr<Div> _viewport{nullptr};
        ScopedSlot           _onMouseMove{}; // On the window, only while dragging the thumb
        float                _dragOffset{0}; // Where the thumb was pressed, from its start
        void updateSkin();
        void dragTo(int windowX, int windowY);
        void removedFromRender() override;
    };
}

