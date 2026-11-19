#include "AppShellPage.hpp"
#include "PageHeader.hpp"
#include "psychic-ui/Div.hpp"
#include "psychic-ui/style/Values.hpp"

namespace psychic_ui {

    AppShellPage::AppShellPage() :
        Div() {
        addClassName("page");
        _defaultStyle.direction(Direction::Column)
            .grow(1)
            .shrink(1);
    }

    void AppShellPage::childAdded(Div &child) {
        Div::childAdded(child);
        if (trackChild(_headers, child) && _headers.size() == 1) {
            headerChanged.emit(true);
        }
    }

    void AppShellPage::childRemoved(Div &child) {
        Div::childRemoved(child);
        if (untrackChild(_headers, child) && _headers.empty()) {
            headerChanged.emit(false);
        }
    }
}
