#include <iostream>
#include <utility>
#include "../components/Menu.hpp"
#include "MenuButton.hpp"

namespace psychic_ui {

    MenuButton::MenuButton(const MenuItem *menuItem) :
        Component(),
        _menuItem(menuItem) {
        tag("MenuButton");

        _mouseChildren = false; // !important
        if (_menuItem) {
            if (_menuItem->danger) {
                addClassName("danger");
            }
            _enabled = _menuItem->enabled;
        }
    }

    const MenuItem *MenuButton::menuItem() const {
        return _menuItem;
    }

    bool MenuButton::selected() const {
        return _selected;
    }

    void MenuButton::updateSelected(const bool selected) {
        if (_selected != selected) {
            _selected = selected;
            invalidateStyle();
        }
    }

    bool MenuButton::active() const {
        return Component::active() || _selected;
    }

    bool MenuButton::checked() const {
        if (const auto *check = dynamic_cast<const CheckMenuItem *>(_menuItem)) {
            return check->checked;
        }
        if (const auto *radio = dynamic_cast<const RadioMenuItem *>(_menuItem)) {
            return radio->checked();
        }
        return false;
    }

    void MenuButton::setColumns(const bool icon, const bool check, const bool checkTrailing) {
        _iconColumn    = icon;
        _checkColumn   = check;
        _checkTrailing = checkTrailing;
        itemChanged();
    }

    void MenuButton::itemChanged() {
        updateClassName("checked", checked());
        if (_skin) {
            _skin->itemChanged();
        }
    }

    void MenuButton::skinChanged() {
        if (_skin) {
            _skin->itemChanged();
        }
    }
}
