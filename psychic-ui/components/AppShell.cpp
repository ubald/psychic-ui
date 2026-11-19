#include "AppShell.hpp"
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

        _page = &_main->add<Div>();
        _page->addClassName("page")
             .direction(Direction::Column)
             .grow(1)
             .shrink(1);

        // The sidebar's header holds macOS's controls; Windows's are ours to place, after it on a change
        SidebarHeader &header = _sidebar->header();
        header.controls().onChange([this](WindowControlsStyle /*style*/) { placeWindowControls(); });
        header.windowControlsToggled.subscribe([this](bool /*shown*/) { placeWindowControls(); });
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
    }

    void AppShell::addedToRender() {
        Div::addedToRender();
        // At the window's top right, the controls' room around their buttons moves the window
        window()->addDragArea(_sidebar->header().controls().shared_from_this());
    }
}
