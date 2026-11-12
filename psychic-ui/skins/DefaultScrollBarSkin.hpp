#pragma once

#include "psychic-ui/Skin.hpp"
#include "psychic-ui/components/ScrollBar.hpp"

namespace psychic_ui {
    class DefaultScrollBarSkin : public ScrollBarSkin {
    public:
        DefaultScrollBarSkin();
    protected:
        ScrollDirection      _direction{ScrollDirection::Vertical};
        std::shared_ptr<Div> _track{nullptr};
        std::shared_ptr<Div> _thumb{nullptr};
        ScopedSlot           _onMouseMove{}; // On the window, only while dragging
        int                  dragOffset{0};
        void added() override;
        void removedFromRender() override;
        void styleUpdated() override;
        void updateScrollBar(bool enabled, float scrollPosition, float contentRatio) override;
        void setupDirection();
    };
}

