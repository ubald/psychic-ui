#include "Menu.hpp"
#include <psychicui/components/MenuButton.hpp>
#include <psychicui/Window.hpp>

namespace psychicui {

    Menu::Menu(const std::vector<std::shared_ptr<MenuItem>> &items) :
        Div(),
        menuContainer(
            std::make_shared<DataContainer<std::shared_ptr<MenuItem>>>(
                items,
                [this](const std::shared_ptr<MenuItem> &item) { return getItemDiv(item); }
            )) {
        setTag("Menu");
        _inlineStyle->set(position, "absolute");
        add(menuContainer);
    }

    std::shared_ptr<Div> Menu::getItemDiv(const std::shared_ptr<MenuItem> &item) {
        auto simple = std::dynamic_pointer_cast<SimpleMenuItem>(item);
        if (simple) {
            auto simpleMenuButton = std::make_shared<MenuButton>(simple.get());
            simpleMenuButton->addClassName("simple");
            return simpleMenuButton;
        }

        auto sub = std::dynamic_pointer_cast<SubMenuItem>(item);
        if (sub) {
            auto subMenuButton = std::make_shared<MenuButton>(sub.get());
            subMenuButton->addClassName("submenu");
            subMenuButton->onMouseOver(
                [this, sub, subMenuButton]() {
                    if (subMenu) {
                        remove(subMenu);
                    }
                    int x, y;
                    subMenuButton->localToGlobal(x, y, subMenuButton->getWidth());
                    int lx, ly;
                    globalToLocal(lx, ly, x, y);
                    subMenu = add<Menu>(sub->items);
                    subMenu->style()
                           ->set(left, lx)
                           ->set(top, ly);
                    for (auto &child: menuContainer->children()) {
                        auto menuButton = std::dynamic_pointer_cast<MenuButton>(child);
                        if (menuButton) {
                            menuButton->setSelected(menuButton->menuItem() == sub.get());
                        }
                    }
                }

            );
            return subMenuButton;
        }

        return nullptr;
    }
}
