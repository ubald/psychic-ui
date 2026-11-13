#include "Menu.hpp"
#include <psychic-ui/components/MenuButton.hpp>
#include <psychic-ui/Window.hpp>

namespace psychic_ui {

    Menu::Menu(const std::vector<std::shared_ptr<MenuItem>> &items) :
        Div(),
        menuContainer(
            std::make_shared<DataContainer<std::shared_ptr<MenuItem>>>(
                items,
                [this](const std::shared_ptr<MenuItem> &item) { return getItemDiv(item); }
            )) {
        tag("Menu");
        _inlineStyle.position(Position::Absolute);
        add(menuContainer);
    }

    std::shared_ptr<Div> Menu::getItemDiv(const std::shared_ptr<MenuItem> &item) {
        auto simple = std::dynamic_pointer_cast<SimpleMenuItem>(item);
        if (simple) {
            auto simpleMenuButton = std::make_shared<MenuButton>(simple.get());
            simpleMenuButton->addClassName("simple");
            simpleMenuButton->onClick([this, simple](){
                if (simple->callback) {
                    simple->callback();
                }
                window()->closeMenu();
            });
            return simpleMenuButton;
        }

        auto sub = std::dynamic_pointer_cast<SubMenuItem>(item);
        if (sub) {
            auto subMenuButton = std::make_shared<MenuButton>(sub.get());
            subMenuButton->addClassName("submenu");
            // The button owns this slot: a raw pointer to it, as a shared_ptr would keep it alive forever
            subMenuButton->onMouseEnter(
                [this, sub, button = subMenuButton.get()]() {
                    if (subMenu) {
                        remove(*subMenu);
                    }
                    int x, y;
                    button->localToGlobal(x, y, button->getWidth());
                    int lx, ly;
                    globalToLocal(lx, ly, x, y);
                    subMenu = &add<Menu>(sub->items)
                        .left(lx)
                        .top(ly);
                    for (auto &child: menuContainer->children()) {
                        auto menuButton = std::dynamic_pointer_cast<MenuButton>(child);
                        if (menuButton) {
                            menuButton->selected(menuButton->menuItem() == sub.get());
                        }
                    }
                }

            );
            return subMenuButton;
        }

        return nullptr;
    }
}
