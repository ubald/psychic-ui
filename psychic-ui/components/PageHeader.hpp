#pragma once

#include <string>
#include <string_view>
#include "psychic-ui/Div.hpp"
#include "Label.hpp"

namespace psychic_ui {
    class Icon;

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
        // Children: our child list owns them
        Icon  *_icon{nullptr};
        Label *_title{nullptr};

        void updateIcon(std::string_view icon);
    };
}
