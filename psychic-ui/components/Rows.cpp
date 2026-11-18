#include <algorithm>
#include "Rows.hpp"
#include "psychic-ui/Window.hpp"

namespace psychic_ui {

    // region Row

    SelectableRow::SelectableRow() :
        Div() {
        tag("SelectableRow");
        _defaultStyle.direction(Direction::Row)
            .alignItems(Align::Center)
            .shrink(0);

        clicked.subscribe(
            [this]() {
                if (!_enabled) {
                    return;
                }
                if (selectsOnClick()) {
                    updateSelected(true);
                } else if (_owner) {
                    _owner->updateTabStop(this); // Tab goes on from here, until the focus leaves the group
                }
                if (window()) {
                    window()->requestFocus(this); // The keyboard goes on from here; no ring for a click
                }
            }
        );
    }

    SelectableRow::~SelectableRow() {
        if (_owner) {
            _owner->forget(this);
        }
    }

    void SelectableRow::removed() {
        if (_owner) {
            _owner->forget(this);
            _owner = nullptr;
        }
        Div::removed();
    }

    RowGroup *SelectableRow::group() const {
        for (const Div *div = parent(); div; div = div->parent()) {
            if (const auto *group = dynamic_cast<const RowGroup *>(div)) {
                return const_cast<RowGroup *>(group);
            }
        }
        return nullptr;
    }

    void SelectableRow::added() {
        Div::added();
        if (RowGroup *rows = group()) {
            rows->rowAdded(this);
        }
    }

    void SelectableRow::updateSelected(const bool selected) {
        RowGroup *rows = group();
        if (!rows) {
            setSelected(selected); // Not in a group yet: only itself
            return;
        }
        if (selected) {
            rows->updateSelection(this);
        } else if (rows->selection() == this) {
            rows->updateSelection(nullptr);
        }
    }

    void SelectableRow::setSelected(const bool selected) {
        if (selected == _selected) {
            return;
        }
        _selected = selected;
        if (_selected) {
            addClassName(selectionClass());
        } else {
            removeClassName(selectionClass());
        }
    }

    // endregion

    // region Group

    RowGroup::RowGroup() :
        Div() {
        tag("RowGroup");

        keyPressed.subscribe(
            [this](const Key key, const Mod mod) {
                if (!_enabled) {
                    return;
                }
                // The row with the focus (the focus is in the group, maybe deeper), else the selection
                SelectableRow *row = nullptr;
                for (SelectableRow *candidate: rows()) {
                    if (candidate->hasFocus()) {
                        row = candidate;
                        break;
                    }
                }
                handleRowKey(row ? row : _selection, key, mod);
            }
        );

        // Where the keyboard only moves the focus, Tab comes back to the selection
        blurred.subscribe(
            [this]() {
                if (!selectionFollowsFocus()) {
                    updateTabStop();
                }
            }
        );
    }

    RowGroup::~RowGroup() {
        // Our rows go after us
        for (SelectableRow *row: RowGroup::rows()) {
            row->_owner = nullptr;
        }
    }

    namespace {
        void collectRows(const Div &div, std::vector<SelectableRow *> &rows) {
            for (const auto &child: div.children()) {
                if (auto row = std::dynamic_pointer_cast<SelectableRow>(child)) {
                    rows.push_back(row.get());
                }
                collectRows(*child, rows);
            }
        }
    }

    std::vector<SelectableRow *> RowGroup::rows() const {
        std::vector<SelectableRow *> rows{};
        collectRows(*this, rows);
        return rows;
    }

    int RowGroup::selectedIndex() const {
        if (!_selection) {
            return -1;
        }
        const auto all = rows();
        const auto it  = std::ranges::find(all, _selection);
        return it == all.end() ? -1 : static_cast<int>(std::distance(all.begin(), it));
    }

    void RowGroup::updateSelection(SelectableRow *row) {
        if (row == _selection) {
            return;
        }
        if (_selection) {
            _selection->setSelected(false);
        }
        _selection = row;
        if (_selection) {
            _selection->setSelected(true);
            _selection->_owner = this;
        }
        updateTabStop(_selection);
        selectionUpdated();
    }

    void RowGroup::updateSelection(const int index) {
        const auto all = rows();
        updateSelection(index >= 0 && index < static_cast<int>(all.size()) ? all[static_cast<std::size_t>(index)] : nullptr);
    }

    void RowGroup::rowAdded(SelectableRow *row) {
        row->_owner = this; // Until it leaves: it tells us then
        if (!_tabStop) {
            updateTabStop(row);
        } else if (row != _tabStop) {
            row->_focusEnabled = false;
        }
        if (row->_selected && row != _selection) {
            row->_selected = false; // Selected before it joined: it becomes the group's selection
            updateSelection(row);
        }
    }

    void RowGroup::updateTabStop(SelectableRow *row) {
        if (!row) {
            row = _selection;
        }
        if (!row) {
            const auto all = rows();
            const auto it  = std::ranges::find_if(all, [](SelectableRow *r) { return r->enabled(); });
            row = it != all.end() ? *it : all.empty() ? nullptr : all.front();
        }
        if (row == _tabStop) {
            return;
        }
        if (_tabStop) {
            _tabStop->_focusEnabled = false;
        }
        _tabStop = row;
        if (_tabStop) {
            _tabStop->_focusEnabled = true;
            _tabStop->_owner = this;
        }
    }

    void RowGroup::forget(SelectableRow *row) {
        if (_reordering) {
            return;
        }
        if (_selection == row) {
            _selection = nullptr;
            selectionUpdated();
        }
        if (_tabStop == row) {
            _tabStop = nullptr; // Another one takes its place when one is next needed
        }
    }

    void RowGroup::reorder(Div &container, const std::vector<std::shared_ptr<Div>> &children) {
        Window *win     = window();
        Div    *focused = nullptr;
        for (const auto &child: children) {
            if (child->hasFocus()) {
                focused = child.get();
            }
        }
        const bool visible = win && win->focusVisible();
        _reordering = true;
        container.removeAll();
        for (const auto &child: children) {
            container.add(child);
        }
        _reordering = false;
        if (focused && win) {
            win->requestFocus(focused, visible);
        }
    }

    bool RowGroup::handleRowKey(SelectableRow *row, const Key key, const Mod /*mod*/) {
        auto all = rows();
        std::erase_if(all, [row](SelectableRow *other) { return other != row && (!other->enabled() || !other->visible()); });
        if (all.empty()) {
            return false;
        }
        const auto it = std::ranges::find(all, row);
        switch (key) {
            case Key::DOWN:
                if (it == all.end()) {
                    moveTo(all.front());
                } else if (std::next(it) != all.end()) {
                    moveTo(*std::next(it));
                }
                return true;
            case Key::UP:
                if (it == all.end()) {
                    moveTo(all.back());
                } else if (it != all.begin()) {
                    moveTo(*std::prev(it));
                }
                return true;
            case Key::HOME:
                moveTo(all.front());
                return true;
            case Key::SPACE:
                if (it != all.end()) {
                    moveTo(row);
                }
                return true;
            case Key::END:
                moveTo(all.back());
                return true;
            default:
                return false;
        }
    }

    void RowGroup::moveTo(SelectableRow *row) {
        if (!row) {
            return;
        }
        if (selectionFollowsFocus()) {
            updateSelection(row);
        } else {
            updateTabStop(row); // Focused rows take the focus ring: it has to be the Tab stop
        }
        if (window()) {
            window()->requestFocus(row, true);
        }
        scrollIntoView(*row);
    }

    void RowGroup::scrollIntoView(Div &div) {
        Div *viewport = div.parent();
        while (viewport && viewport->computedStyle().overflow() != Overflow::Scroll) {
            viewport = viewport->parent();
        }
        if (!viewport) {
            return;
        }
        // Where the div is in the viewport, whose own scroll position is left out
        int divY = 0;
        int dummy = 0;
        div.localToGlobal(dummy, divY);
        int viewportY = 0;
        viewport->localToGlobal(dummy, viewportY);
        const int top    = divY - (viewportY - viewport->scrollY());
        const int bottom = top + div.getHeight();
        if (top < 0) {
            // Up to the start when only what is above the first rows would be left out (a border, a header)
            const int fromStart = top - viewport->scrollY();
            viewport->scrollY(fromStart < div.getHeight() ? 0 : viewport->scrollY() - top);
        } else if (bottom > viewport->getHeight()) {
            viewport->scrollY(viewport->scrollY() - (bottom - viewport->getHeight()));
        }
    }

    // endregion
}
