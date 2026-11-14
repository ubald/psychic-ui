#pragma once

#include <string>
#include "psychic-ui/Div.hpp"
#include "Icon.hpp"
#include "Label.hpp"
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
        // Children, which we own
        Sidebar *_sidebar{nullptr};
        Div     *_main{nullptr};
        Div     *_page{nullptr};
    };

    /**
     * @class PageHeader
     *
     * A page's title bar, the height of the sidebar's header so that their lines meet: an optional icon and the
     * title (the `.icon` and `.title` parts), then what is added to it (a menu bar, actions; a Spacer pushes what
     * follows it to the end).
     */
    class PageHeader : public Div {
    public:
        explicit PageHeader(const std::string &title = "", const std::string &icon = "");

        const std::string &title() const { return _title->text(); }

        template<class Self>
        Self &title(this Self &self, const std::string &title) {
            static_cast<PageHeader &>(self)._title->text(title);
            return self;
        }

        template<class Self>
        Self &icon(this Self &self, detail::StringArg icon) {
            static_cast<PageHeader &>(self).updateIcon(icon.value);
            return self;
        }

    protected:
        // Children, which we own
        Icon  *_icon{nullptr};
        Label *_title{nullptr};

        void updateIcon(std::string_view icon);
    };
}
