#pragma once

#include "psychic-ui/components/TextArea.hpp"
#include "psychic-ui/components/Scroller.hpp"
#include "psychic-ui/components/Text.hpp"
#include "psychic-ui/paint/Box.hpp"
#include "SkinState.hpp"

namespace psychic_ui {
    /**
     * What every text area skin shares: its `.viewport` part, a scroller, around the `.text` part, the view its field
     * drives, at least the viewport's height; both are its own, which its child list owns. A skin set's text area
     * only paints its chrome, behind them.
     */
    class TextAreaSkinBase : public TextAreaSkin {
    public:
        TextAreaSkinBase();

        Text *textView() override;

    protected:
        Scroller *_viewport{nullptr}; // The `.viewport` part
        Text     *_text{nullptr};     // The `.text` part, inside it

        void draw(SkCanvas *canvas) final;

        /**
         * Paint the field behind its text. The default is the Div's own painting.
         */
        virtual void drawChrome(SkCanvas *canvas, const paint::Box &box, SkinState state);
    };
}
