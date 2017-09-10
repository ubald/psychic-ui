#pragma once

#include <string>
#include <functional>
#include <memory>
#include <utility>
#include "DataContainer.hpp"

namespace psychicui {

    class MenuItem {
    public:
        std::string label{};
        std::string shortcut{};

        explicit MenuItem(const std::string &itemLabel, const std::string &itemShortcut = "") :
            label(itemLabel),
            shortcut(itemShortcut) {}

        virtual void please_be_polymorphic() {};
    };

    class SimpleMenuItem : public MenuItem {
    public:
        std::function<void()> callback{nullptr};

        SimpleMenuItem(const std::string &itemLabel, std::function<void()> itemCallback, const std::string &itemShortcut = "") :
            MenuItem(itemLabel, itemShortcut),
            callback(std::move(itemCallback)) {}
    };

    class SubMenuItem : public MenuItem {
    public:
        std::vector<std::shared_ptr<MenuItem>> items{};

        SubMenuItem(const std::string &itemLabel, std::vector<std::shared_ptr<MenuItem>> items, const std::string &itemShortcut = "") :
            MenuItem(itemLabel, itemShortcut),
            items(std::move(items)) {}
    };

    // MENU

    class Menu : public Div {
    public:
        explicit Menu(const std::vector<std::shared_ptr<MenuItem>> &items);

        inline static std::shared_ptr<SimpleMenuItem>
        item(const std::string &label, std::function<void()> callback, const std::string &shortcut = "") {
            return std::make_shared<SimpleMenuItem>(label, callback, shortcut);
        }

        inline static std::shared_ptr<SubMenuItem>
        item(const std::string &label, const std::vector<std::shared_ptr<MenuItem>> &items, const std::string &shortcut = "") {
            return std::make_shared<SubMenuItem>(label, items, shortcut);
        }

    protected:
        std::shared_ptr<Div> getItemDiv(const std::shared_ptr<MenuItem> &item);
        std::shared_ptr<DataContainer<std::shared_ptr<MenuItem>>> menuContainer{};
        std::shared_ptr<Menu> subMenu{};
    };
}
