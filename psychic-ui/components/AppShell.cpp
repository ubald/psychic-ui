#include "AppShell.hpp"

namespace psychic_ui {

    AppShell::AppShell() :
        Div() {
        tag("AppShell");
        _defaultStyle.direction(Direction::Row)
            .grow(1);

        _sidebar = &add<Sidebar>();

        _main = &add<Div>();
        _main->addClassName("main")
             .direction(Direction::Column)
             .grow(1)
             .shrink(1);

        _page = &_main->add<Div>();
        _page->addClassName("page")
             .direction(Direction::Column)
             .grow(1)
             .shrink(1);
    }

    PageHeader::PageHeader(const std::string &title, const std::string &icon) :
        Div() {
        tag("PageHeader");
        _defaultStyle.direction(Direction::Row)
            .alignItems(Align::Center)
            .shrink(0);

        _icon = &add<Icon>(icon);
        _icon->addClassName("icon")
             .display(icon.empty() ? Display::None : Display::Flex);

        _title = &add<Label>(title);
        _title->addClassName("title")
              .grow(1); // Pushes what is added after it to the end
    }

    void PageHeader::updateIcon(const std::string_view icon) {
        _icon->name(icon)
             .display(icon.empty() ? Display::None : Display::Flex);
    }
}
