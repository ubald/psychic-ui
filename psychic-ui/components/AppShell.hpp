#pragma once

#include "psychic-ui/Div.hpp"
#include "AppShellPage.hpp"
#include "Sidebar.hpp"

namespace psychic_ui {

    /**
     * @class AppShell
     *
     * An app's frame, Pivotal's: the sidebar the window's full height on the left, and the page beside it, a panel
     * inset in the frame (the `.main` part, padded by the inset, holds the `.page` part). The frame is the sidebar's
     * color; the page floats on it with rounded corners and clips what it holds.
     *
     * The window controls (the sidebar header's `controls()`) go where their style puts them: macOS's first in the
     * sidebar's header, Windows's at the window's top right, in the `.main` part over the page's top-right corner,
     * which the rules place in the band of the page's header (the shell has the `controls-top-right` class meanwhile,
     * from which the rules leave them room at the end of a `PageHeader` that is a child of the page, and the
     * `page-has-header` class while the page has one). A page without one gets them over its top-right corner, over
     * its content, as tight as their buttons. They move when their style changes, and the
     * header's `windowControls(false)` takes them out in either style. Around Windows's buttons, the window drags
     * while the page has its `PageHeader`; without one, its content under them takes the presses.
     *
     *     auto &shell = app.add<AppShell>();
     *     shell.sidebar().title("Lights");
     *     shell.page().add<PageHeader>("Fixtures", "bulb");
     */
    class AppShell : public Div {
    public:
        AppShell();

        Sidebar &sidebar() const { return *_sidebar; }

        /**
         * The inset page panel, where the app's pages go
         */
        AppShellPage &page() const { return *_page; }

    protected:
        // Children: our child list owns them
        Sidebar      *_sidebar{nullptr};
        Div          *_main{nullptr};
        AppShellPage *_page{nullptr};

        void placeWindowControls();

        /**
         * Whether the room around the window controls moves the window: at the top right, with a `PageHeader` in the
         * page to carry them
         */
        bool controlsDrag() const;
        void addedToRender() override;
    };
}
