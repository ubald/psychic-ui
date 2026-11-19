#include "AppShell.hpp"
#include "AppShellPage.hpp"
#include "Sidebar.hpp"
#include "WindowControls.hpp"
#include "WindowControlsStyle.hpp"
#include "psychic-ui/Div.hpp"
#include "psychic-ui/Window.hpp"
#include "psychic-ui/style/Values.hpp"

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

        _page = &_main->add<AppShellPage>();

        // The sidebar's header holds macOS's controls; Windows's are ours to place, after it on a change
        SidebarHeader &header = _sidebar->header();
        subscribeTo(header.controls().changed, [this](WindowControlsStyle /*style*/) { placeWindowControls(); });
        subscribeTo(header.windowControlsToggled, [this](bool /*shown*/) { placeWindowControls(); });
        subscribeTo(_page->headerChanged, [this](bool /*hasHeader*/) { placeWindowControls(); });
        placeWindowControls();
    }

    void AppShell::placeWindowControls() {
        const SidebarHeader &header   = _sidebar->header();
        WindowControls      &controls = header.controls();
        // Last in the main part, so over the page
        if (header.windowControls() && !windowControlsLead(controls.controlsStyle())) {
            if (controls.parent() != _main) {
                _main->add(controls.shared_from_this());
            }
            addClassName("controls-top-right");
        } else {
            if (controls.parent() == _main) {
                _main->remove(controls);
            }
            removeClassName("controls-top-right");
        }
        // Whether the page has a header to carry them, which gives them its band
        if (_page->hasHeader()) {
            addClassName("page-has-header");
        } else {
            removeClassName("page-has-header");
        }
    }

    void AppShell::addedToRender() {
        Div::addedToRender();
        // At the window's top right, the room around the buttons moves the window while a page header carries it;
        // over a page without one, the content under it takes its presses
        window()->addDragArea(_sidebar->header().controls().shared_from_this(), [this] { return controlsDrag(); });
    }

    bool AppShell::controlsDrag() const {
        return classNames().contains("controls-top-right") && _page->hasHeader();
    }
}
