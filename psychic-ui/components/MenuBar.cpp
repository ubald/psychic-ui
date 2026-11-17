#include "MenuBar.hpp"
#include <psychic-ui/components/Button.hpp>
#include <psychic-ui/Window.hpp>

namespace psychic_ui {

    MenuBar::MenuBar(const std::vector<std::shared_ptr<MenuItem>> &items) :
        DataContainer<std::shared_ptr<MenuItem>>(
            items,
            [this](const std::shared_ptr<MenuItem> &item) { return getItemDiv(item); }
        ) {
        tag("MenuBar");

        _defaultStyle.shrink(0);
    }

    std::shared_ptr<Div> MenuBar::getItemDiv(const std::shared_ptr<MenuItem> &item) {
        auto simple = std::dynamic_pointer_cast<SimpleMenuItem>(item);
        if (simple) {
            return std::make_shared<Button>(simple->label, simple->callback);
        }

        auto sub = std::dynamic_pointer_cast<SubMenuItem>(item);
        if (sub) {
            auto menuItem = std::make_shared<Button>(sub->label);
            menuItem->toggle(true);
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
            menuItem->onMouseEnter(
                [this, sub, button = menuItem.get()]() {
                    if (_menuOpen) {
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

        // Under its trigger; the bar still takes the mouse, so that hovering another trigger switches menus
        Menu &menu = window()->openMenu(sub->items, *menuItemButton, Side::Bottom, PopupAlign::Start);
        menu.passThrough(*this);
        menu.navigatedOut = [this](const int step) { openNeighbor(step); };

        for (auto &child: _children) {
            auto menuButton = std::dynamic_pointer_cast<Button>(child);
            if (menuButton) {
                menuButton->selected(menuButton.get() == menuItemButton);
            }
        }

        currentMenuItem = sub;
        _menuOpen      = true;
    }

    void MenuBar::openNeighbor(const int step) {
        // The triggers that open menus, in order
        std::vector<std::pair<Button *, SubMenuItem *>> triggers{};
        for (std::size_t i = 0; i < _children.size() && i < _data.size(); ++i) {
            auto button = std::dynamic_pointer_cast<Button>(_children[i]);
            auto sub    = std::dynamic_pointer_cast<SubMenuItem>(_data[i]);
            if (button && sub) {
                triggers.emplace_back(button.get(), sub.get());
            }
        }
        const auto count = static_cast<int>(triggers.size());
        for (int i = 0; i < count; ++i) {
            if (triggers[i].second == currentMenuItem) {
                const auto &[button, sub] = triggers[(i + step + count) % count];
                openMenu(button, sub);
                return;
            }
        }
    }

    void MenuBar::addedToRender() {
        DataContainer<std::shared_ptr<MenuItem>>::addedToRender();
        _menuClosedSlot = window()->menuClosed.subscribe(
            [this]() {
                if (!_menuOpen) {
                    return;
                }

                _menuOpen      = false;
                currentMenuItem = nullptr;
                for (auto &child: _children) {
                    auto menuButton = std::dynamic_pointer_cast<Button>(child);
                    if (menuButton) {
                        menuButton->selected(false);
                    }
                }
            }
        );
    }

    void MenuBar::removedFromRender() {
        DataContainer<std::shared_ptr<MenuItem>>::removedFromRender();
        _menuClosedSlot.disconnect();
    }

}
