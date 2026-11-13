#pragma once

#include "psychic-ui/components/ScrollBar.hpp"
#include "SkinState.hpp"

namespace psychic_ui {
    /**
     * What every scroll bar skin shares: the `.track` part, filling the skin, and the `.thumb` part over it, placed
     * along the track from the scroll position and sized from the visible part of the content.
     * Both parts paint themselves from their style (`ScrollBar .thumb { backgroundColor }`).
     */
    class ScrollBarSkinBase : public ScrollBarSkin {
    public:
        ScrollBarSkinBase();

    protected:
        ScrollDirection _direction{ScrollDirection::Vertical};
        Div             *_track{nullptr}; // The `.track` part, a child, which we own
        Div             *_thumb{nullptr}; // The `.thumb` part, a child, which we own

        void added() override;
        void styleUpdated() override;
        void updateScrollBar(bool enabled, float scrollPosition, float contentRatio) override;
        void setupDirection();
    };
}
