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
}
