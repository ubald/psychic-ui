#include "Menu.hpp"
#include <psychicui/components/Button.hpp>

namespace psychicui {

    Menu::Menu(const std::vector<std::shared_ptr<MenuItem>> &items) :
        DataContainer<std::shared_ptr<MenuItem>>(
            items,
            [this](const std::shared_ptr<MenuItem> &item) { return getItemDiv(item); }
        ) {
        setTag("Menu");
        _inlineStyle->set(position, "absolute");
    }

    std::shared_ptr<Div> Menu::getItemDiv(const std::shared_ptr<MenuItem> &item) const {
        return std::make_shared<Button>(item->label);
    }
}