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
            auto button = std::make_shared<Button>(sub->label);
            button->onClick(
                [this, sub, button]() {
                    std::cout << "MNU" << std::endl;
                    int x, y;
                    button->getGlobalPosition(x, y);
                    y += button->getHeight();
                    window()->openMenu(sub->items, x, y);
                }
            );
            return button;
        }

        return nullptr;
    }

}