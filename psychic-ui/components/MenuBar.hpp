#pragma once

#include <psychic-ui/components/DataContainer.hpp>
#include <psychic-ui/components/Button.hpp>
#include <psychic-ui/components/Menu.hpp>
#include <psychic-ui/Window.hpp>

namespace psychic_ui {
    class MenuBar : public DataContainer<std::shared_ptr<MenuItem>> {
    public:
        explicit MenuBar(const std::vector<std::shared_ptr<MenuItem>> &items);
    protected:
        std::shared_ptr<Div> getItemDiv(const std::shared_ptr<MenuItem> &item);
        void openMenu(Button *menuItemButton, SubMenuItem *sub);

        /**
         * Open the menu before (-1) or after (1) the open one, wrapping around (Left and Right in a menu)
         */
        void openNeighbor(int step);

        void addedToRender() override;
        void removedFromRender() override;
        ScopedSlot _menuClosedSlot{}; // On the window, while we're in it
        bool       _menuOpen{false};
        MenuItem *currentMenuItem{nullptr};
    };
}
