#pragma once

#include "psychic-ui/Component.hpp"

namespace psychic_ui {
    class ScrollBar;

    class ScrollBarSkin : public Skin<ScrollBar> {
    public:
        ScrollBarSkin() : Skin<ScrollBar>() {
            tag("ScrollBarSkin");
        }

        virtual void updateScrollBar(bool enabled, float scrollPosition, float contentRatio) = 0;
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
        void updateSkin();
    };
}

