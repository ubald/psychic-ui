#pragma once

#include "psychic-ui/Component.hpp"

namespace psychic_ui {
    class ScrollBar;

    class ScrollBarSkin : public Skin<ScrollBar> {
    public:
        ScrollBarSkin() : Skin<ScrollBar>() {
            setTag("ScrollBarSkin");

        }

        virtual void updateScrollBar(bool enabled, float scrollPosition, float contentRatio) = 0;
    };

    enum ScrollDirection {
        Horizontal,
        Vertical
    };

    class ScrollBar: public Component<ScrollBarSkin> {
    public:
        explicit ScrollBar(const std::shared_ptr<Div> &viewport, ScrollDirection direction = Vertical);
        Div *viewport() const;
        const ScrollDirection direction() const;
        void scrollPercentX(float scrollPercentX);
        void scrollPercentY(float scrollPercentY);
    protected:
        ScrollDirection      _direction{Vertical};
        std::shared_ptr<Div> _viewport{nullptr};
        void updateSkin();
    };
}

