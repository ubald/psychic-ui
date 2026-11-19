#include <algorithm>
#include <iterator>
#include <memory>
#include <string>
#include <string_view>
#include <vector>
#include "Icon.hpp"
#include "Label.hpp"
#include "Rows.hpp"
#include "Sidebar.hpp"
#include "WindowControls.hpp"
#include "WindowControlsStyle.hpp"
#include "psychic-ui/Div.hpp"
#include "psychic-ui/Window.hpp"
#include "psychic-ui/psychic-ui.hpp"
#include "psychic-ui/style/Values.hpp"

namespace psychic_ui {

    // region Item

    SidebarItem::SidebarItem(const std::string &label, const std::string &icon) :
        SelectableRow() {
        tag("SidebarItem");
        _mouseChildren = false;

        _icon = &add<Icon>(icon);
        _icon->addClassName("icon")
             .display(icon.empty() ? Display::None : Display::Flex);
        _label = &add<Label>(label);
        _label->addClassName("label");
        _count = &add<Label>();
        _count->addClassName("count")
              .display(Display::None);
        _disclosure = &add<Icon>("disclosure");
        _disclosure->addClassName("disclosure")
                   .display(Display::None);

        clicked.subscribe(
            [this](const MouseEvent &click) {
                if (_enabled && onDisclosure(click)) {
                    toggle();
                }
            }
        );
    }

    Sidebar *SidebarItem::sidebar() {
        return dynamic_cast<Sidebar *>(group());
    }

    SidebarItem &SidebarItem::item(const std::string &label, const std::string &icon) {
        if (!_children) {
            _children = std::make_shared<Div>();
            _children->addClassName("children")
                     .display(_expanded ? Display::Flex : Display::None);
            _disclosure->display(Display::Flex);
            placeChildren();
        }
        auto &child       = _children->add<SidebarItem>(label, icon);
        child._parentItem = this;
        return child;
    }

    std::vector<SidebarItem *> SidebarItem::items() const {
        std::vector<SidebarItem *> items{};
        if (_children) {
            for (const auto &child: _children->children()) {
                if (auto item = std::dynamic_pointer_cast<SidebarItem>(child)) {
                    items.push_back(item.get());
                }
            }
        }
        return items;
    }

    void SidebarItem::added() {
        SelectableRow::added();
        placeChildren();
    }

    void SidebarItem::placeChildren() {
        Div *container = parent();
        if (_children && container && _children->parent() != container) {
            container->add(static_cast<unsigned int>(container->childIndex(this) + 1), _children);
        }
    }

    void SidebarItem::removed() {
        if (_children && _children->parent()) {
            _children->parent()->remove(*_children);
        }
        SelectableRow::removed();
    }

    void SidebarItem::updateExpanded(const bool expanded) {
        if (expanded == _expanded) {
            return;
        }
        _expanded = expanded;
        _disclosure->name(_expanded ? "chevron-down" : "disclosure");
        if (_children) {
            _children->display(_expanded ? Display::Flex : Display::None);
        }
        if (_expanded) {
            addClassName("expanded");
        } else {
            removeClassName("expanded");
            if (Sidebar *bar = sidebar()) {
                bar->collapsed(*this);
            }
        }
    }

    bool SidebarItem::onDisclosure(const MouseEvent &click) const {
        // From the chevron's left edge to the row's end; a click made by a key is the row's
        return !click.fromKeyboard && _disclosure->visible() && _children && !_children->children().empty()
               && click.x >= static_cast<float>(_disclosure->x());
    }

    void SidebarItem::updateIcon(const std::string_view icon) {
        _icon->name(icon)
             .display(icon.empty() ? Display::None : Display::Flex);
    }

    void SidebarItem::updateCount(const std::string &count) {
        _count->text(count)
              .display(count.empty() ? Display::None : Display::Flex);
    }

    // endregion

    // region Section

    SidebarSection::SidebarSection(const std::string &heading) :
        Div() {
        tag("SidebarSection");
        _defaultStyle.shrink(0);
        _heading = &add<Label>(heading);
        _heading->addClassName("heading")
                .display(heading.empty() ? Display::None : Display::Flex);
    }

    // endregion

    // region Header

    SidebarHeader::SidebarHeader() :
        Div() {
        tag("SidebarHeader");
        _defaultStyle.direction(Direction::Row)
            .alignItems(Align::Center)
            .shrink(0);

        // Ours in macOS's style, which puts them at the start of the bar; they follow a change of style
        _controls = std::make_shared<WindowControls>();
        subscribeTo(_controls->changed, [this](WindowControlsStyle /*style*/) { placeWindowControls(); });

        _identity = &add<Div>();
        _identity->addClassName("identity")
                 .direction(Direction::Column)
                 .grow(1)
                 .shrink(1);

        _brand = &_identity->add<Div>();
        _brand->addClassName("brand");

        _title = &_brand->add<Label>();
        _title->addClassName("title");

        _brand->add<Icon>("chevron-down").addClassName("chevron");

        placeWindowControls();
    }

    void SidebarHeader::updateWindowControls(const bool shown) {
        if (shown == _windowControls) {
            return;
        }
        _windowControls = shown;
        placeWindowControls();
        windowControlsToggled.emit(shown);
    }

    void SidebarHeader::placeWindowControls() {
        // First in the header while shown in macOS's style; out of it otherwise (Windows's are the shell's to place)
        if (_windowControls && windowControlsLead(_controls->controlsStyle())) {
            if (_controls->parent() != this) {
                add(0, _controls);
            }
        } else if (_controls->parent() == this) {
            remove(*_controls);
        }
    }

    Div &SidebarHeader::mark() {
        if (!_mark) {
            _mark = &_brand->add(0, std::make_shared<Div>());
            _mark->addClassName("mark");
        }
        return *_mark;
    }

    Div &SidebarHeader::details() {
        if (!_details) {
            _details = &_identity->add<Div>();
            _details->addClassName("details");
        }
        return *_details;
    }

    void SidebarHeader::setTitle(const std::string &title) {
        _title->text(title);
    }

    void SidebarHeader::setMark(const std::string &mark) {
        if (!mark.empty()) {
            if (!_letters) {
                _letters = &this->mark().add<Label>();
            }
            _letters->text(mark);
            return;
        }
        if (_letters) {
            _mark->remove(*_letters);
            _letters = nullptr;
        }
        if (_mark && _mark->childCount() == 0) {
            _brand->remove(*_mark);
            _mark = nullptr;
        }
    }

    void SidebarHeader::addedToRender() {
        Div::addedToRender();
        // Lets the window manager or compositor move the window from here (on Wayland too); the controls still
        // take their presses
        window()->addDragArea(shared_from_this());
    }

    // endregion

    // region Sidebar

    Sidebar::Sidebar() :
        RowGroup() {
        tag("Sidebar");
        _defaultStyle.direction(Direction::Column)
            .shrink(0);
        _header   = &add<SidebarHeader>();
        _sections = &add<Div>();
        _sections->addClassName("sections")
                 .grow(1)
                 .shrink(1);
    }

    SidebarSection &Sidebar::section(const std::string &heading) {
        return _sections->add<SidebarSection>(heading);
    }

    namespace {
        void collectItems(const Div &div, std::vector<SidebarItem *> &items) {
            for (const auto &child: div.children()) {
                if (auto item = std::dynamic_pointer_cast<SidebarItem>(child)) {
                    items.push_back(item.get());
                } else {
                    collectItems(*child, items);
                }
            }
        }
    }

    std::vector<SidebarItem *> Sidebar::items() const {
        std::vector<SidebarItem *> items{};
        collectItems(*_sections, items);
        return items;
    }

    namespace {
        /**
         * Whether an item isn't hidden in a collapsed parent
         */
        bool shown(const SidebarItem &item) {
            for (const SidebarItem *parent = item.parentItem(); parent; parent = parent->parentItem()) {
                if (!parent->expanded()) {
                    return false;
                }
            }
            return true;
        }

        /**
         * The item itself if it shows, else its outermost collapsed parent, which shows
         */
        const SidebarItem *shownItem(const SidebarItem &item) {
            const SidebarItem *found = &item;
            for (const SidebarItem *parent = item.parentItem(); parent; parent = parent->parentItem()) {
                if (!parent->expanded()) {
                    found = parent;
                }
            }
            return found;
        }

        /**
         * Whether an item is nested in another one, at any depth
         */
        bool nestedIn(const SidebarItem *item, const SidebarItem &ancestor) {
            for (const SidebarItem *parent = item ? item->parentItem() : nullptr; parent; parent = parent->parentItem()) {
                if (parent == &ancestor) {
                    return true;
                }
            }
            return false;
        }
    }

    std::vector<SelectableRow *> Sidebar::rows() const {
        std::vector<SelectableRow *> rows{};
        std::ranges::copy_if(items(), std::back_inserter(rows), [](const SidebarItem *item) { return shown(*item); });
        return rows;
    }

    SidebarItem *Sidebar::current() const {
        // The selection, found among the items (a row of another kind is none of them)
        const auto all = items();
        const auto it  = std::ranges::find(all, _selection);
        return it != all.end() ? *it : nullptr;
    }

    void Sidebar::selectionUpdated() {
        const SidebarItem *item = current();
        for (SidebarItem *parent = item ? item->parentItem() : nullptr; parent; parent = parent->parentItem()) {
            parent->expanded(true);
        }
        for (SidebarItem *other: items()) {
            if (nestedIn(item, *other)) {
                other->addClassName("has-current");
            } else {
                other->removeClassName("has-current");
            }
        }
        changed.emit(current());
    }

    SelectableRow *Sidebar::restingTabStop() const {
        const SidebarItem *item = current();
        if (!item) {
            return _selection;
        }
        // Among the rows the keyboard moves along, which it shows in
        const auto all = rows();
        const auto it  = std::ranges::find(all, shownItem(*item));
        return it != all.end() ? *it : _selection;
    }

    void Sidebar::collapsed(SidebarItem &item) {
        bool refocus = false;
        for (const SidebarItem *other: items()) {
            refocus = refocus || (other->hasFocus() && nestedIn(other, item));
        }
        if (Window *win = window(); win && refocus) {
            win->requestFocus(&item, win->focusVisible());
        }
        if (nestedIn(dynamic_cast<SidebarItem *>(_tabStop), item)) {
            updateTabStop(&item);
        }
    }

    bool Sidebar::handleRowKey(SelectableRow *row, const Key key, const Mod mod) {
        auto *item = dynamic_cast<SidebarItem *>(row);
        switch (key) {
            case Key::ENTER:
            case Key::KP_ENTER:
            case Key::SPACE:
                // Clicks the focused item, at its center
                if (row && row->enabled() && row->hasFocus()) {
                    MouseEvent click = row->mouseEventAtCenter();
                    click.button     = MouseButton::LEFT;
                    row->clicked.emit(click);
                }
                return true;
            case Key::RIGHT:
                if (item && !item->items().empty()) {
                    item->expanded(true);
                }
                return true;
            case Key::LEFT:
                if (item && !item->items().empty() && item->expanded()) {
                    item->expanded(false);
                } else if (item && item->parentItem()) {
                    moveTo(item->parentItem());
                }
                return true;
            default:
                return RowGroup::handleRowKey(row, key, mod);
        }
    }

    // endregion
}
