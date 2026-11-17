#pragma once

#include "psychic-ui/components/SearchField.hpp"
#include "psychic-ui/components/Icon.hpp"
#include "psychic-ui/paint/Box.hpp"
#include "SkinState.hpp"

namespace psychic_ui {
    /**
     * What every search field skin shares, its parts, in a row: the `.icon` (a search Icon, which the skin owns),
     * then the field's `.input` and `.clear` button (which the field owns, so that they keep their state when the
     * skin changes). A skin set's search field paints the field's chrome around them; its rules make the input
     * borderless (`SearchField .input`).
     */
    class SearchFieldSkinBase : public SearchFieldSkin {
    public:
        SearchFieldSkinBase();

    protected:
        Icon *_icon{nullptr}; // The `.icon` part, a child, which we own

        void addedToComponent() override;
        void removedFromComponent() override;
        void draw(SkCanvas *canvas) final;

        /**
         * Paint the field behind its parts. The default is the Div's own painting.
         */
        virtual void drawChrome(SkCanvas *canvas, const paint::Box &box, SkinState state);
    };
}
