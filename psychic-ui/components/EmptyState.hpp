#pragma once

#include <string>
#include "psychic-ui/Div.hpp"

namespace psychic_ui {
    class Button;
    class Icon;
    class Label;
    class Text;

    /**
     * @class EmptyState
     *
     * Fills a page or pane that has nothing to show, and says what to do about it, centered: an `.icon` (on a
     * `.icon-tile`), the `.title`, the `.description`, and an optional `.action` button
     *
     *     add<EmptyState>("inbox", "No messages", "New messages show up here.")
     *         .action("Compose").onClick([] { ... });
     */
    class EmptyState : public Div {
    public:
        explicit EmptyState(const std::string &icon = "", const std::string &title = "", const std::string &description = "");

        std::string title() const;

        template<class Self>
        Self &title(this Self &self, const std::string &title) {
            static_cast<EmptyState &>(self).updateTitle(title);
            return self;
        }

        std::string description() const;

        template<class Self>
        Self &description(this Self &self, const std::string &description) {
            static_cast<EmptyState &>(self).updateDescription(description);
            return self;
        }

        /**
         * The icon, by its registered name; "" for none
         */
        template<class Self>
        Self &icon(this Self &self, detail::StringArg icon) {
            static_cast<EmptyState &>(self).updateIcon(icon.value);
            return self;
        }

        /**
         * Add the action (a primary button, labeled), or relabel it, and return it
         */
        Button &action(const std::string &label);

        /**
         * The action button, or nullptr when there is none
         */
        Button *actionButton() const { return _action; }

    protected:
        // Children: our child list owns them
        Div    *_tile{nullptr};
        Icon   *_icon{nullptr};
        Label  *_title{nullptr};
        Text   *_description{nullptr};
        Button *_action{nullptr};

        void updateTitle(const std::string &title);
        void updateDescription(const std::string &description);
        void updateIcon(std::string_view icon);
    };
}
