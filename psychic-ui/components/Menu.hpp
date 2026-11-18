#pragma once

#include <string>
#include <functional>
#include <memory>
#include <utility>
#include <vector>
#include "psychic-ui/Popup.hpp"

namespace psychic_ui {

    class MenuButton;

    /**
     * A row of a menu, as data: what menus (and menu bars) are made of. Menus are made again from their items
     * every time they open, so an item keeps its state (a check mark) between openings.
     */
    class MenuItem {
    public:
        std::string label{};
        std::string shortcut{};
        std::string icon{};   // A registered icon's name, shown before the label; "" for none
        bool        enabled{true};
        bool        danger{false}; // An action that destroys: in the danger color

        explicit MenuItem(const std::string &itemLabel, const std::string &itemShortcut = "", const std::string &itemIcon = "") :
            label(itemLabel),
            shortcut(itemShortcut),
            icon(itemIcon) {}

        virtual ~MenuItem() = default;
    };

    /**
     * An item that calls `callback`, then closes its menu
     */
    class SimpleMenuItem : public MenuItem {
    public:
        std::function<void()> callback{nullptr};

        SimpleMenuItem(const std::string &itemLabel, std::function<void()> itemCallback, const std::string &itemShortcut = "",
                       const std::string &itemIcon = "") :
            MenuItem(itemLabel, itemShortcut, itemIcon),
            callback(std::move(itemCallback)) {}
    };

    /**
     * An item that opens a menu of its own beside it
     */
    class SubMenuItem : public MenuItem {
    public:
        std::vector<std::shared_ptr<MenuItem>> items{};

        SubMenuItem(const std::string &itemLabel, std::vector<std::shared_ptr<MenuItem>> items, const std::string &itemShortcut = "",
                    const std::string &itemIcon = "") :
            MenuItem(itemLabel, itemShortcut, itemIcon),
            items(std::move(items)) {}
    };

    /**
     * An item with a check mark, toggled when chosen (the menu stays open), then `callback` with the new state
     */
    class CheckMenuItem : public MenuItem {
    public:
        bool                      checked{false};
        std::function<void(bool)> callback{nullptr};

        CheckMenuItem(const std::string &itemLabel, bool itemChecked, std::function<void(bool)> itemCallback,
                      const std::string &itemShortcut = "", const std::string &itemIcon = "") :
            MenuItem(itemLabel, itemShortcut, itemIcon),
            checked(itemChecked),
            callback(std::move(itemCallback)) {}
    };

    /**
     * The value a group of radio items chooses, and what is called when it changes
     */
    class MenuRadioGroup {
    public:
        std::string                             value{};
        std::function<void(const std::string &)> callback{nullptr};

        explicit MenuRadioGroup(std::string groupValue = "", std::function<void(const std::string &)> groupCallback = nullptr) :
            value(std::move(groupValue)),
            callback(std::move(groupCallback)) {}
    };

    /**
     * One of a group's values, checked while it is the group's: choosing it makes it the group's value (the menu
     * stays open, unless `closes`)
     */
    class RadioMenuItem : public MenuItem {
    public:
        std::shared_ptr<MenuRadioGroup> group{};
        std::string                     value{};
        bool                            closes{false};

        RadioMenuItem(std::shared_ptr<MenuRadioGroup> itemGroup, const std::string &itemLabel, std::string itemValue,
                      const std::string &itemShortcut = "", const std::string &itemIcon = "") :
            MenuItem(itemLabel, itemShortcut, itemIcon),
            group(std::move(itemGroup)),
            value(std::move(itemValue)) {}

        bool checked() const { return group && group->value == value; }
    };

    /**
     * A hairline between groups of items
     */
    class SeparatorMenuItem : public MenuItem {
    public:
        SeparatorMenuItem() : MenuItem("") {}
    };

    /**
     * A small muted heading over the items after it
     */
    class GroupMenuItem : public MenuItem {
    public:
        explicit GroupMenuItem(const std::string &itemLabel) : MenuItem(itemLabel) {}
    };

    using MenuItems = std::vector<std::shared_ptr<MenuItem>>;

    /**
     * @class Menu
     *
     * A list of actions in a popup (Pivotal's): rows with an icon, a label and a shortcut (`MenuButton`), separators
     * (`.separator`), group labels (`.group-label`), checkable and radio items with a check mark in a column before
     * the label, and submenus, opened beside their row (hover, Right or Enter) and flipping to the other side at the
     * window's edge. When a row has an icon, every row keeps the icon column, so labels line up.
     *
     * One row at a time is highlighted (`selected`, `:active` in stylesheets), by the mouse or the keyboard: Up and
     * Down move it (wrapping, skipping disabled rows), Home and End go to the ends, Right or Enter opens a submenu
     * (Left or Escape closes it), Enter or Space chooses. Open one with `Window::openMenu`, or as a context menu with
     * `Window::openContextMenu`; it is modal, closes on a press outside it and on Escape, and gives the focus back.
     */
    class Menu : public Popup {
    public:
        explicit Menu(const MenuItems &items);

        /**
         * An item that calls `callback`, with an optional shortcut shown after its label and an icon before it
         */
        inline static std::shared_ptr<SimpleMenuItem>
        item(const std::string &label, std::function<void()> callback, const std::string &shortcut = "", const std::string &icon = "") {
            return std::make_shared<SimpleMenuItem>(label, std::move(callback), shortcut, icon);
        }

        /**
         * An item opening a submenu of these items
         */
        inline static std::shared_ptr<SubMenuItem>
        item(const std::string &label, const MenuItems &items, const std::string &shortcut = "", const std::string &icon = "") {
            return std::make_shared<SubMenuItem>(label, items, shortcut, icon);
        }

        /**
         * A checkable item
         */
        inline static std::shared_ptr<CheckMenuItem>
        check(const std::string &label, bool checked, std::function<void(bool)> callback = nullptr, const std::string &shortcut = "",
              const std::string &icon = "") {
            return std::make_shared<CheckMenuItem>(label, checked, std::move(callback), shortcut, icon);
        }

        /**
         * A radio item: one of `group`'s values
         */
        inline static std::shared_ptr<RadioMenuItem>
        radio(const std::shared_ptr<MenuRadioGroup> &group, const std::string &label, const std::string &value,
              const std::string &shortcut = "", const std::string &icon = "") {
            return std::make_shared<RadioMenuItem>(group, label, value, shortcut, icon);
        }

        inline static std::shared_ptr<SeparatorMenuItem> separator() {
            return std::make_shared<SeparatorMenuItem>();
        }

        /**
         * A group label
         */
        inline static std::shared_ptr<GroupMenuItem> group(const std::string &label) {
            return std::make_shared<GroupMenuItem>(label);
        }

        /**
         * The rows that can be highlighted (not separators or group labels), in order
         */
        const std::vector<MenuButton *> &rows() const { return _rows; }

        /**
         * The highlighted row's index in `rows()`, or -1
         */
        int highlighted() const { return _highlighted; }

        /**
         * Highlight a row (-1 for none). With `openSubmenu`, a submenu row opens its submenu, as hovering does.
         */
        void highlight(int index, bool openSubmenu = false);

        /**
         * Choose a row, as clicking it does: call it, toggle it, or open its submenu (and move into it)
         */
        void choose(int index);

        /**
         * Put the check marks after the labels instead of before them (a select's list)
         */
        void checksAfter(bool after);

        /**
         * The submenu open beside it, or nullptr
         */
        Menu *submenu() const;

        /**
         * The menu at the root of its chain of submenus
         */
        Menu *root();

        /**
         * Called with -1 or 1 when Left or Right goes past the root menu, so that a menu bar opens its neighbor
         */
        std::function<void(int)> navigatedOut{nullptr};

    protected:
        MenuItems                 _items{};
        std::vector<MenuButton *> _rows{};      // Children: our child list owns them
        int                       _highlighted{-1};
        std::weak_ptr<Menu>       _submenu{};
        int                       _submenuRow{-1};
        std::weak_ptr<Menu>       _closedSubmenu{};     // The last one closed, while it fades out
        int                       _closedSubmenuRow{-1};
        bool                      _iconColumn{false};
        bool                      _checkColumn{false};
        bool                      _takeFocus{true}; // A submenu opened by hovering leaves the focus where it is

        void opened() override;
        void openSubmenu(int index, bool focus);
        void closeSubmenu();
        void move(int step);
        bool handleKey(Key key);
        void refreshRows();

        /**
         * Close the whole chain, then call what was chosen
         */
        void finish(const std::function<void()> &action);
    };
}
