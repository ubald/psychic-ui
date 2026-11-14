#pragma once

#include "psychic-ui/components/MenuButton.hpp"
#include "psychic-ui/components/Icon.hpp"
#include "psychic-ui/components/Label.hpp"
#include "psychic-ui/skins/paint/Box.hpp"
#include "SkinState.hpp"

namespace psychic_ui {
    /**
     * What every menu item skin shares: the `.check` part (a check mark, drawn when the item is checked, displayed when
     * its menu keeps a check column), the `.icon` part (displayed when the item has one or its menu keeps an icon
     * column), the `.label` part, the `.shortcut` part (not displayed when empty), then the `.indicator` part (the
     * check mark after the label instead, in a select's list), from the item. A submenu's skin adds an `.arrow` part.
     */
    class MenuButtonSkinBase : public MenuButtonSkin {
    public:
        MenuButtonSkinBase();

    protected:
        Icon  *_check{nullptr};     // The `.check` part, a child, which we own
        Icon  *_icon{nullptr};     // The `.icon` part, a child, which we own
        Label *_label{nullptr};    // The `.label` part, a child, which we own
        Label *_shortcut{nullptr}; // The `.shortcut` part, a child, which we own
        Icon  *_indicator{nullptr}; // The `.indicator` part, a child, which we own

        void added() override;
        void itemChanged() override;
        void draw(SkCanvas *canvas) final;

        /**
         * Paint the item behind its parts. The default is the Div's own painting.
         */
        virtual void drawChrome(SkCanvas *canvas, const paint::Box &box, SkinState state);
    };
}
