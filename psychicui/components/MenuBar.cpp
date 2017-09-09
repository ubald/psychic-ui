#include "MenuBar.hpp"
#include <psychicui/components/Button.hpp>
#include <psychicui/Window.hpp>

namespace psychicui {

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
            menuItem->onClick(
                [this, sub, menuItem]() {
                    int x, y;
                    menuItem->getGlobalPosition(x, y);
                    y += menuItem->getHeight();
                    window()->openMenu(sub->items, x, y);
                }
            );
            return menuItem;
        }

        return nullptr;
    }

}