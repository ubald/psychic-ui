#include "MenuBar.hpp"
#include <psychic-ui/components/Button.hpp>
#include <psychic-ui/Window.hpp>

namespace psychic_ui {

    MenuBar::MenuBar(const std::vector<std::shared_ptr<MenuItem>> &items) :
        DataContainer<std::shared_ptr<MenuItem>>(
            items,
            [this](const std::shared_ptr<MenuItem> &item) { return getItemDiv(item); }
        ) {
        setTag("MenuBar");

        _defaultStyle
            ->set(shrink, 0)
            ->set(justifyContent, "stretch");
    }

    std::shared_ptr<Div> MenuBar::getItemDiv(const std::shared_ptr<MenuItem> &item) {
        auto simple = std::dynamic_pointer_cast<SimpleMenuItem>(item);
        if (simple) {
            return std::make_shared<Button>(simple->label, simple->callback);
        }

        auto sub = std::dynamic_pointer_cast<SubMenuItem>(item);
        if (sub) {
            auto menuItem = std::make_shared<Button>(sub->label);
            menuItem->setToggle(true);
            // The button owns these slots: a raw pointer to it, as a shared_ptr would keep it alive forever
            menuItem->onClick(
                [this, sub, button = menuItem.get()]() {
                    if (sub.get() == currentMenuItem) {
                        window()->closeMenu();
                    } else {
                        openMenu(button, sub.get());
                    }
                }
            );
            menuItem->onMouseOver(
                [this, sub, button = menuItem.get()]() {
                    if (menuOpened) {
                        openMenu(button, sub.get());
                    }
                }
            );
            return menuItem;
        }

        return nullptr;
    }

    void MenuBar::openMenu(Button *menuItemButton, SubMenuItem *sub) {
        if (sub == currentMenuItem) {
            return;
        }

        int x, y;
        menuItemButton->localToGlobal(x, y);
        y += menuItemButton->getHeight();
        window()->openMenu(sub->items, x, y);

        if (!menuOpened) {
            window()->menuContainer()->addMouseEnabledDiv(shared_from_this());
        }

        for (auto &child: _children) {
            auto menuButton = std::dynamic_pointer_cast<Button>(child);
            if (menuButton) {
                menuButton->setSelected(menuButton.get() == menuItemButton);
            }
        }

        currentMenuItem = sub;
        menuOpened      = true;
    }

    void MenuBar::addedToRender() {
        DataContainer<std::shared_ptr<MenuItem>>::addedToRender();
        menuClosed = subscribeTo(
            window()->onMenuClosed, [this]() {
                if (!menuOpened) {
                    return;
                }

                menuOpened      = false;
                currentMenuItem = nullptr;
                window()->menuContainer()->removeMouseEnabledDiv(shared_from_this());
                for (auto &child: _children) {
                    auto menuButton = std::dynamic_pointer_cast<Button>(child);
                    if (menuButton) {
                        menuButton->setSelected(false);
                    }
                }
            }
        );
    }

    void MenuBar::removedFromRender() {
        DataContainer<std::shared_ptr<MenuItem>>::removedFromRender();
        unsubscribeFrom(menuClosed);
        menuClosed = nullptr;
    }

}
