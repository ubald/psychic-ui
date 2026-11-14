#pragma once

#include <string>
#include <functional>
#include <memory>
#include <utility>
#include "DataContainer.hpp"

namespace psychic_ui {

    class MenuItem {
    public:
        std::string label{};
        std::string shortcut{};
        std::string icon{}; // A registered icon's name, shown before the label; "" for none

        explicit MenuItem(const std::string &itemLabel, const std::string &itemShortcut = "", const std::string &itemIcon = "") :
            label(itemLabel),
            shortcut(itemShortcut),
            icon(itemIcon) {}

        virtual void please_be_polymorphic() {};
    };

    class SimpleMenuItem : public MenuItem {
    public:
        std::function<void()> callback{nullptr};

        SimpleMenuItem(const std::string &itemLabel, std::function<void()> itemCallback, const std::string &itemShortcut = "",
                       const std::string &itemIcon = "") :
            MenuItem(itemLabel, itemShortcut, itemIcon),
            callback(std::move(itemCallback)) {}
    };

    class SubMenuItem : public MenuItem {
    public:
        std::vector<std::shared_ptr<MenuItem>> items{};

        SubMenuItem(const std::string &itemLabel, std::vector<std::shared_ptr<MenuItem>> items, const std::string &itemShortcut = "",
                    const std::string &itemIcon = "") :
            MenuItem(itemLabel, itemShortcut, itemIcon),
            items(std::move(items)) {}
    };

    // MENU

    class Menu : public Div {
    public:
        explicit Menu(const std::vector<std::shared_ptr<MenuItem>> &items);

        /**
         * An item that calls `callback`, with an optional shortcut shown after its label and an icon before it
         */
        inline static std::shared_ptr<SimpleMenuItem>
        item(const std::string &label, std::function<void()> callback, const std::string &shortcut = "", const std::string &icon = "") {
            return std::make_shared<SimpleMenuItem>(label, callback, shortcut, icon);
        }

        inline static std::shared_ptr<SubMenuItem>
        item(const std::string &label, const std::vector<std::shared_ptr<MenuItem>> &items, const std::string &shortcut = "",
             const std::string &icon = "") {
            return std::make_shared<SubMenuItem>(label, items, shortcut, icon);
        }

    protected:
        std::shared_ptr<Div> getItemDiv(const std::shared_ptr<MenuItem> &item);
        std::shared_ptr<DataContainer<std::shared_ptr<MenuItem>>> menuContainer{};
        Menu *subMenu{nullptr}; // A child, which we own
    };
}
