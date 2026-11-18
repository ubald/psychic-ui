#include <algorithm>
#include "Sidebar.hpp"
#include "psychic-ui/Window.hpp"

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
    }

    Sidebar *SidebarItem::sidebar() {
        return dynamic_cast<Sidebar *>(group());
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

    SidebarItem *Sidebar::current() const {
        // The selection, found among the items (a row of another kind is none of them)
        const auto all = items();
        const auto it  = std::ranges::find(all, _selection);
        return it != all.end() ? *it : nullptr;
    }

    void Sidebar::selectionUpdated() {
        changed.emit(current());
    }

    bool Sidebar::handleRowKey(SelectableRow *row, const Key key, const Mod mod) {
        if (key == Key::ENTER || key == Key::KP_ENTER || key == Key::SPACE) {
            // Clicks the focused item, at its center
            if (row && row->enabled() && row->hasFocus()) {
                MouseEvent click = row->mouseEventAtCenter();
                click.button     = MouseButton::LEFT;
                row->clicked.emit(click);
            }
            return true;
        }
        return RowGroup::handleRowKey(row, key, mod);
    }

    // endregion
}
