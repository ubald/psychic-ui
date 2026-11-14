#pragma once

#include "psychic-ui/components/MenuButton.hpp"
#include "psychic-ui/components/Icon.hpp"
#include "psychic-ui/components/Label.hpp"
#include "psychic-ui/skins/paint/Box.hpp"
#include "SkinState.hpp"

namespace psychic_ui {
    /**
     * What every menu item skin shares: the `.icon` part, the `.label` part, then the `.shortcut` part (not displayed
     * when empty), from the item. A submenu's skin adds an `.arrow` part. When the item has no icon, its `.icon` part
     * draws nothing and has the `blank` class, so the skin set's rules decide whether it keeps its column: the menu
     * gives its rows the `icon-column` class when any of them has an icon.
     */
    class MenuButtonSkinBase : public MenuButtonSkin {
    public:
        MenuButtonSkinBase();

    protected:
        Icon  *_icon{nullptr};     // The `.icon` part, a child, which we own
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
