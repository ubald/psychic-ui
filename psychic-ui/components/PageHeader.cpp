#include "PageHeader.hpp"
#include "Icon.hpp"

namespace psychic_ui {

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
        _title->addClassName("title");
    }

    void PageHeader::updateIcon(const std::string_view icon) {
        _icon->name(icon)
             .display(icon.empty() ? Display::None : Display::Flex);
    }
}
