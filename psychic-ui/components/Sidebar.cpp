#include <algorithm>
#include "Sidebar.hpp"
#include "psychic-ui/Window.hpp"

namespace psychic_ui {

    // region Item

    SidebarItem::SidebarItem(const std::string &label, const std::string &icon) :
        Div() {
        tag("SidebarItem");
        _defaultStyle.direction(Direction::Row)
            .alignItems(Align::Center)
            .shrink(0);
        _mouseChildren = false;
        _focusEnabled  = true; // Tab reaches it; Enter or Space picks it, the arrows move along the sidebar

        _icon = &add<Icon>(icon);
        _icon->addClassName("icon")
             .display(icon.empty() ? Display::None : Display::Flex);
        _label = &add<Label>(label);
        _label->addClassName("label");
        _count = &add<Label>();
        _count->addClassName("count")
              .display(Display::None);

        clicked.subscribe(
            [this]() {
                if (_enabled && _selectable) {
                    updateCurrent(true);
                }
            }
        );

        keyPressed.subscribe(
            [this](const Key key, const Mod /*modifiers*/) {
                if (!_enabled) {
                    return;
                }
                if (key == Key::ENTER || key == Key::KP_ENTER || key == Key::SPACE) {
                    MouseEvent click = mouseEventAtCenter();
                    click.button     = MouseButton::LEFT;
                    clicked.emit(click);
                    return;
                }
                if (key != Key::UP && key != Key::DOWN) {
                    return;
                }
                Sidebar *bar = sidebar();
                if (!bar || !window()) {
                    return;
                }
                // The next or previous item that can take the focus
                auto items = bar->items();
                std::erase_if(items, [this](SidebarItem *item) { return item != this && (!item->enabled() || !item->visible()); });
                auto it = std::find(items.begin(), items.end(), this);
                if (it == items.end()) {
                    return;
                }
                if (key == Key::DOWN && std::next(it) != items.end()) {
                    window()->requestFocus(*std::next(it), true);
                } else if (key == Key::UP && it != items.begin()) {
                    window()->requestFocus(*std::prev(it), true);
                }
            }
        );
    }

    SidebarItem::~SidebarItem() {
        if (_owner) {
            _owner->forget(this);
        }
    }

    Sidebar *SidebarItem::sidebar() const {
        for (const Div *div = parent(); div; div = div->parent()) {
            if (auto bar = dynamic_cast<const Sidebar *>(div)) {
                return const_cast<Sidebar *>(bar);
            }
        }
        return nullptr;
    }

    void SidebarItem::updateIcon(const std::string_view icon) {
        _icon->name(icon)
             .display(icon.empty() ? Display::None : Display::Flex);
    }

    void SidebarItem::updateCount(const std::string &count) {
        _count->text(count)
              .display(count.empty() ? Display::None : Display::Flex);
    }

    void SidebarItem::updateCurrent(const bool current) {
        Sidebar *bar = sidebar();
        if (!bar) {
            setCurrent(current); // Not in a sidebar yet: only itself
            return;
        }
        if (current) {
            bar->updateCurrent(this);
        } else if (bar->current() == this) {
            bar->updateCurrent(nullptr);
        }
    }

    void SidebarItem::setCurrent(const bool current) {
        if (current == _current) {
            return;
        }
        _current = current;
        if (_current) {
            addClassName("current");
        } else {
            removeClassName("current");
        }
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

        _controls = &add<WindowControls>();

        _brand = &add<Div>();
        _brand->addClassName("brand");

        auto &mark = _brand->add<Div>();
        mark.addClassName("mark")
            .display(Display::None);
        _mark = &mark.add<Label>();

        _title = &_brand->add<Label>();
        _title->addClassName("title");

        _brand->add<Icon>("chevron-down").addClassName("chevron");
    }

    void SidebarHeader::setTitle(const std::string &title) {
        _title->text(title);
    }

    void SidebarHeader::setMark(const std::string &mark) {
        _mark->text(mark);
        _mark->parent()->display(mark.empty() ? Display::None : Display::Flex);
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
        Div() {
        tag("Sidebar");
        _defaultStyle.direction(Direction::Column)
            .shrink(0);
        _header   = &add<SidebarHeader>();
        _sections = &add<Div>();
        _sections->addClassName("sections")
                 .grow(1)
                 .shrink(1);
    }

    Sidebar::~Sidebar() {
        if (_current) {
            _current->_owner = nullptr; // It goes after us
        }
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

    void Sidebar::updateCurrent(SidebarItem *item) {
        if (item == _current) {
            return;
        }
        if (_current) {
            _current->setCurrent(false);
            _current->_owner = nullptr;
        }
        _current = item;
        if (_current) {
            _current->setCurrent(true);
            _current->_owner = this;
        }
        changed.emit(_current);
    }

    void Sidebar::forget(SidebarItem *item) {
        if (_current == item) {
            _current = nullptr;
        }
    }

    // endregion
}
