#pragma once

#include "psychic-ui/components/SplitterDivider.hpp"
#include "psychic-ui/paint/Box.hpp"
#include "SkinState.hpp"

namespace psychic_ui {
    /**
     * What every splitter divider skin shares, its part: the `.line`, centered across the divider, which the skin
     * set's rules size and paint (`SplitterDivider.horizontal .line { width }`, `SplitterDivider:pressed .line
     * { backgroundColor }`). It stretches along the boundary.
     */
    class SplitterDividerSkinBase : public SplitterDividerSkin {
    public:
        SplitterDividerSkinBase();

    protected:
        Div *_line{nullptr}; // The `.line` part, a child: our child list owns it

        void draw(SkCanvas *canvas) final;

        /**
         * Paint the divider under its line. The default is the Div's own painting.
         */
        virtual void drawChrome(SkCanvas *canvas, const paint::Box &box, SkinState state);
    };
}
