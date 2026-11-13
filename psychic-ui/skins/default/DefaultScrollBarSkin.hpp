#pragma once

#include "psychic-ui/signals/Slot.hpp"
#include "psychic-ui/skins/base/ScrollBarSkinBase.hpp"

namespace psychic_ui {
    /**
     * The default scroll bar: the track and thumb parts, painted from their style
     */
    class DefaultScrollBarSkin : public ScrollBarSkinBase {
    public:
        DefaultScrollBarSkin();
    protected:
        ScopedSlot _onMouseMove{}; // On the window, only while dragging
        int        dragOffset{0};
        void removedFromRender() override;
    };
}
