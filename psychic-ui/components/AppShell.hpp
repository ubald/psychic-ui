#pragma once

#include "psychic-ui/Div.hpp"
#include "Sidebar.hpp"

namespace psychic_ui {

    /**
     * @class AppShell
     *
     * An app's frame, Pivotal's: the sidebar the window's full height on the left, and the page beside it, a panel
     * inset in the frame (the `.main` part, padded by the inset, holds the `.page` part). The frame is the sidebar's
     * color; the page floats on it with rounded corners and clips what it holds.
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
        Div &page() const { return *_page; }

    protected:
        // Children: our child list owns them
        Sidebar *_sidebar{nullptr};
        Div     *_main{nullptr};
        Div     *_page{nullptr};
    };
}
