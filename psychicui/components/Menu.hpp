#pragma once

#include <string>
#include <functional>
#include <memory>
#include "DataContainer.hpp"

namespace psychicui {

    class MenuItem {
    public:
        std::string label{};
        std::string shortcut{};

        MenuItem(std::string itemLabel, std::string itemShortcut = "") :
            label(itemLabel),
            shortcut(itemShortcut) {}

        virtual void dummy() {};
    };

    class SimpleMenuItem : public MenuItem {
    public:
        std::function<void()> callback{nullptr};

        SimpleMenuItem(std::string itemLabel, std::function<void()> itemCallback, std::string itemShortcut = "") :
            MenuItem(itemLabel, itemShortcut),
            callback(itemCallback) {}
    };

    class SubMenuItem : public MenuItem {
    public:
        std::vector<std::shared_ptr<MenuItem>> items{};

        SubMenuItem(std::string itemLabel, const std::vector<std::shared_ptr<MenuItem>> &items, std::string itemShortcut = "") :
            MenuItem(itemLabel, itemShortcut),
            items(items) {}
    };

    // MENU

    class Menu : public DataContainer<std::shared_ptr<MenuItem>> {
    public:
        explicit Menu(const std::vector<std::shared_ptr<MenuItem>> &items);

        inline static std::shared_ptr<SimpleMenuItem> item(std::string label, std::function<void()> callback, std::string shortcut = "") {
            return std::make_shared<SimpleMenuItem>(label, callback, shortcut);
        }

        inline static std::shared_ptr<SubMenuItem> item(std::string label, const std::vector<std::shared_ptr<MenuItem>> &items, std::string shortcut = "") {
            return std::make_shared<SubMenuItem>(label, items, shortcut);
        }

    protected:
        std::shared_ptr<Div> getItemDiv(const std::shared_ptr<MenuItem> &item) const;
    };
}
