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
        void openMenu(std::shared_ptr<Button> menuItemButton, SubMenuItem *sub);

        void addedToRender() override;
        void removedFromRender() override;
        Window::MenuClosedSlot menuClosed{nullptr};
        bool                   menuOpened{false};
        MenuItem *currentMenuItem{nullptr};
    };
}
