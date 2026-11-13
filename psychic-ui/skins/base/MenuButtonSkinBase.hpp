#pragma once

#include "psychic-ui/components/MenuButton.hpp"
#include "psychic-ui/components/Label.hpp"
#include "psychic-ui/skins/paint/Box.hpp"
#include "SkinState.hpp"

namespace psychic_ui {
    /**
     * What every menu item skin shares: the `.label` part, then the `.shortcut` part, from the item.
     * A submenu's skin adds an `.arrow` part after them.
     */
    class MenuButtonSkinBase : public MenuButtonSkin {
    public:
        MenuButtonSkinBase();

    protected:
        Label *_label{nullptr};    // The `.label` part, a child, which we own
        Label *_shortcut{nullptr}; // The `.shortcut` part, a child, which we own

        void added() override;
        void draw(SkCanvas *canvas) final;

        /**
         * Paint the item behind its parts. The default is the Div's own painting.
         */
        virtual void drawChrome(SkCanvas *canvas, const paint::Box &box, SkinState state);
    };
}
