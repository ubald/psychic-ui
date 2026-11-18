#pragma once

#include "psychic-ui/components/MenuButton.hpp"
#include "psychic-ui/components/Icon.hpp"
#include "psychic-ui/components/Label.hpp"
#include "psychic-ui/paint/Box.hpp"
#include "SkinState.hpp"

namespace psychic_ui {
    /**
     * What every menu item skin shares: the `.check` part (a check mark, drawn when the item is checked, displayed when
     * its menu keeps a check column), the `.icon` part (displayed when the item has one or its menu keeps an icon
     * column), the `.label` part, the `.shortcut` part (not displayed when empty), the `.indicator` part (the
     * check mark after the label instead, in a select's list), then the `.arrow` part (a chevron, displayed when the
     * item opens a submenu), from the item.
     */
    class MenuButtonSkinBase : public MenuButtonSkin {
    public:
        MenuButtonSkinBase();

    protected:
        Icon  *_check{nullptr};     // The `.check` part, a child: our child list owns it
        Icon  *_icon{nullptr};      // The `.icon` part, a child: our child list owns it
        Label *_label{nullptr};     // The `.label` part, a child: our child list owns it
        Label *_shortcut{nullptr};  // The `.shortcut` part, a child: our child list owns it
        Icon  *_indicator{nullptr}; // The `.indicator` part, a child: our child list owns it
        Icon  *_arrow{nullptr};     // The `.arrow` part, a child: our child list owns it

        void added() override;
        void itemChanged() override;
        void draw(SkCanvas *canvas) final;

        /**
         * Paint the item behind its parts. The default is the Div's own painting.
         */
        virtual void drawChrome(SkCanvas *canvas, const paint::Box &box, SkinState state);
    };
}
