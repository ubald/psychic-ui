#include <algorithm>
#include "Menu.hpp"
#include "Label.hpp"
#include <psychic-ui/components/MenuButton.hpp>
#include <psychic-ui/Window.hpp>

namespace psychic_ui {

    Menu::Menu(const MenuItems &items) :
        Popup(),
        _items(items) {
        tag("Menu");
        modal(true);
        trapFocus(true);
        passesContextPress(true); // A right press elsewhere opens the context menu there
        _focusEnabled = true; // It takes the keys, its rows are highlighted
        _defaultStyle.direction(Direction::Column);

        // When a row has an icon (or a check mark), every row keeps the column
        const bool icons  = _iconColumn = std::ranges::any_of(items, [](const auto &item) { return !item->icon.empty(); });
        const bool checks = _checkColumn = std::ranges::any_of(items, [](const auto &item) {
            return dynamic_cast<CheckMenuItem *>(item.get()) || dynamic_cast<RadioMenuItem *>(item.get());
        });

        for (const auto &item: items) {
            if (dynamic_cast<SeparatorMenuItem *>(item.get())) {
                add<Div>().addClassName("separator");
                continue;
            }
            if (dynamic_cast<GroupMenuItem *>(item.get())) {
                add<Label>(item->label).addClassName("group-label");
                continue;
            }

            auto &row = add<MenuButton>(item.get());
            if (dynamic_cast<SubMenuItem *>(item.get())) {
                row.addClassName("submenu");
            } else if (dynamic_cast<CheckMenuItem *>(item.get())) {
                row.addClassName("checkable");
            } else if (dynamic_cast<RadioMenuItem *>(item.get())) {
                row.addClassName("radio");
            } else {
                row.addClassName("simple");
            }
            row.setColumns(icons, checks, false);

            const int index = static_cast<int>(_rows.size());
            _rows.push_back(&row);
            // The rows own these slots, and the menu owns the rows
            row.onMouseEnter([this, index]() { highlight(index, true); });
            row.onMouseExit(
                [this, index]() {
                    // A submenu's row stays highlighted while its submenu is open
                    if (_highlighted == index && !submenu()) {
                        highlight(-1);
                    }
                }
            );
            row.onClick([this, index]() { choose(index); });
        }

        keyPressed.subscribe([this](const Key key, const Mod /*modifiers*/) { handleKey(key); });
    }

    void Menu::opened() {
        Window *w = window();
        if (!w || !_takeFocus) {
            return;
        }
        // Opened from the keyboard (or moved into with it): its first row is highlighted
        const bool keyboard = w->focusVisible();
        w->requestFocus(this, keyboard);
        if (keyboard && _highlighted < 0) {
            move(1);
        }
    }

    void Menu::checksAfter(const bool after) {
        for (MenuButton *row: _rows) {
            row->setColumns(_iconColumn, _checkColumn, after);
        }
    }

    Menu *Menu::submenu() const {
        return _submenu.lock().get();
    }

    Menu *Menu::root() {
        auto *owner = dynamic_cast<Menu *>(this->owner());
        return owner ? owner->root() : this;
    }

    void Menu::highlight(const int index, const bool openSubmenu) {
        if (index < -1 || index >= static_cast<int>(_rows.size())) {
            return;
        }
        _highlighted = index;
        for (std::size_t i = 0; i < _rows.size(); ++i) {
            _rows[i]->selected(static_cast<int>(i) == index);
        }
        if (!openSubmenu) {
            return;
        }
        if (index >= 0 && _rows[index]->enabled() && dynamic_cast<const SubMenuItem *>(_rows[index]->menuItem())) {
            this->openSubmenu(index, false);
        } else {
            closeSubmenu();
        }
    }

    void Menu::move(const int step) {
        const int count = static_cast<int>(_rows.size());
        if (count == 0) {
            return;
        }
        int index = _highlighted;
        for (int tries = 0; tries < count; ++tries) {
            if (index < 0) {
                index = step > 0 ? 0 : count - 1;
            } else {
                index = (index + step + count) % count;
            }
            if (_rows[index]->enabled()) {
                highlight(index);
                return;
            }
        }
    }

    void Menu::openSubmenu(const int index, const bool focus) {
        auto *row = _rows[index];
        const auto *sub = dynamic_cast<const SubMenuItem *>(row->menuItem());
        if (!sub) {
            return;
        }
        Window *w = window();
        if (!w) {
            return;
        }

        auto open = _submenu.lock();
        if (open && _submenuRow == index) {
            if (focus) {
                w->requestFocus(open.get(), true);
                if (open->_highlighted < 0) {
                    open->move(1);
                }
            }
            return;
        }
        closeSubmenu();

        // Back onto the row whose submenu is still fading out: it comes back from where it is
        if (auto closed = _closedSubmenu.lock(); closed && closed->exiting() && _closedSubmenuRow == index) {
            closed->_takeFocus = focus;
            _submenu    = closed;
            _submenuRow = index;
            highlight(index);
            w->overlay().open(closed);
            if (focus) {
                w->requestFocus(closed.get(), w->focusVisible());
                if (closed->_highlighted < 0) {
                    closed->move(1);
                }
            }
            return;
        }

        // Beside its row, its first row level with it: up by the menu's padding and border, which are the first
        // child's offset. Not modal, so that the mouse still reaches this menu.
        const float inset = _children.empty() ? 0.0f : static_cast<float>(_children.front()->y());
        auto        menu  = std::make_shared<Menu>(sub->items);
        menu->_takeFocus = focus;
        menu->anchor(*row)
            .side(Side::Right)
            .alignment(PopupAlign::Start)
            .sideOffset(4)
            .alignOffset(-inset)
            .owner(*this)
            .modal(false);
        menu->addClassName("submenu");
        menu->navigatedOut = nullptr;
        _submenu    = menu;
        _submenuRow = index;
        highlight(index);
        w->overlay().open(menu);
        if (focus) {
            // Moved into from the keyboard (or a click): at its first row
            w->requestFocus(menu.get(), w->focusVisible());
            if (menu->_highlighted < 0) {
                menu->move(1);
            }
        }
    }

    void Menu::closeSubmenu() {
        if (auto open = _submenu.lock()) {
            _submenu.reset();
            open->close();
            _closedSubmenu    = open;
            _closedSubmenuRow = _submenuRow;
        }
        _submenuRow = -1;
    }

    void Menu::finish(const std::function<void()> &action) {
        // Closing frees this menu: nothing of it is used after
        auto call = action;
        root()->close();
        if (call) {
            call();
        }
    }

    void Menu::choose(const int index) {
        if (index < 0 || index >= static_cast<int>(_rows.size()) || !_rows[index]->enabled()) {
            return;
        }
        auto *item = const_cast<MenuItem *>(_rows[index]->menuItem()); // Ours: the menu holds its items
        if (auto *simple = dynamic_cast<SimpleMenuItem *>(item)) {
            finish(simple->callback);
        } else if (dynamic_cast<SubMenuItem *>(item)) {
            openSubmenu(index, true);
        } else if (auto *check = dynamic_cast<CheckMenuItem *>(item)) {
            check->checked = !check->checked;
            highlight(index);
            refreshRows();
            if (check->callback) {
                check->callback(check->checked);
            }
        } else if (auto *radio = dynamic_cast<RadioMenuItem *>(item)) {
            const bool changed = radio->group && radio->group->value != radio->value;
            if (radio->group) {
                radio->group->value = radio->value;
            }
            highlight(index);
            refreshRows();
            auto group = radio->group; // The callback may outlive the menu
            const std::string value = radio->value;
            if (radio->closes) {
                finish(changed && group && group->callback ? std::function<void()>([group, value]() { group->callback(value); })
                                                           : std::function<void()>{});
            } else if (changed && group && group->callback) {
                group->callback(value);
            }
        }
    }

    void Menu::refreshRows() {
        for (MenuButton *row: _rows) {
            row->itemChanged();
        }
    }

    bool Menu::handleKey(const Key key) {
        switch (key) {
            case Key::DOWN:
                move(1);
                return true;
            case Key::UP:
                move(-1);
                return true;
            case Key::HOME:
                _highlighted = -1;
                move(1);
                return true;
            case Key::END:
                _highlighted = -1;
                move(-1);
                return true;
            case Key::RIGHT:
                if (_highlighted >= 0 && dynamic_cast<const SubMenuItem *>(_rows[_highlighted]->menuItem())) {
                    openSubmenu(_highlighted, true);
                } else if (const Menu *top = root(); top->navigatedOut) {
                    auto out = top->navigatedOut;
                    out(1);
                }
                return true;
            case Key::LEFT:
                if (dynamic_cast<Menu *>(owner())) {
                    close(); // The focus goes back to the menu it came from
                } else if (navigatedOut) {
                    auto out = navigatedOut;
                    out(-1);
                }
                return true;
            case Key::ENTER:
            case Key::KP_ENTER:
            case Key::SPACE:
                if (_highlighted >= 0) {
                    choose(_highlighted);
                }
                return true;
            default:
                return false;
        }
    }
}
