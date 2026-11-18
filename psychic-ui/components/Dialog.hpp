#pragma once

#include <functional>
#include <string>
#include "psychic-ui/Popup.hpp"
#include "Button.hpp"
#include "Icon.hpp"
#include "Label.hpp"
#include "Text.hpp"

namespace psychic_ui {

    /**
     * @class Dialog
     *
     * A modal panel in the middle of the window over a backdrop (Pivotal's): a header with an icon, the title and a
     * close button (`.header`: `.icon`, `.title`, `.close`), a body (`.body`: the description, `.description`, then
     * what is added to `body()`), and a footer of actions (`.footer`, after a hairline; not displayed without one).
     * The panel (`.panel`) is its surface: a press on the backdrop closes it (`dismissible(false)` keeps it), and so
     * does Escape. Once laid out it focuses its first field (a focusable div of the body that isn't a button), else
     * its first button (body, then footer), else keeps the focus on its panel; `initialFocus(div)` picks another.
     * Tab stays inside it; when it closes the focus goes back to what had it.
     *
     *     auto &dialog = window.overlay().open<Dialog>("Rename cue").description("…");
     *     dialog.body().add<TextInput>();
     *     dialog.action("Cancel");
     *     dialog.action("Rename", [] { … }, "primary");
     */
    class Dialog : public Popup {
    public:
        explicit Dialog(const std::string &title = "");

        const std::string &title() const { return _title->text(); }

        template<class Self>
        Self &title(this Self &self, const std::string &title) {
            static_cast<Dialog &>(self)._title->text(title);
            return self;
        }

        /**
         * The icon before the title; "" for none
         */
        template<class Self>
        Self &icon(this Self &self, detail::StringArg icon) {
            static_cast<Dialog &>(self).updateIcon(icon.value);
            return self;
        }

        /**
         * The text at the top of the body, in the muted color; "" for none
         */
        template<class Self>
        Self &description(this Self &self, const std::string &description) {
            static_cast<Dialog &>(self).updateDescription(description);
            return self;
        }

        /**
         * Wider: Pivotal's medium dialog (560 px) instead of the small one (400 px), the `wide` class
         */
        template<class Self>
        Self &wide(this Self &self, detail::BoolArg wide) {
            static_cast<Dialog &>(self).updateClassName("wide", wide.value);
            return self;
        }

        /**
         * Whether a press on the backdrop closes it (true)
         */
        template<class Self>
        Self &dismissible(this Self &self, detail::BoolArg dismissible) {
            return self.dismissOnOutsidePress(dismissible.value);
        }

        /**
         * The div it focuses once it is laid out, instead of its first field or button
         */
        template<class Self>
        Self &initialFocus(this Self &self, Div &div) {
            static_cast<Dialog &>(self)._initialFocus = div.weak_from_this();
            return self;
        }

        /**
         * What it focuses once laid out: `initialFocus`'s div, else its first field, else its first button, else
         * nullptr (the panel keeps the focus)
         */
        Div *initialFocus() const;

        Div &panel() const { return *_panel; }
        Div &body() const { return *_body; }
        Div &footer() const { return *_footer; }

        /**
         * Add a button to the footer, a skin variant ("secondary", "primary", "danger"...): clicking it calls
         * `callback`, after closing the dialog unless `closes` is false
         */
        Button &action(const std::string &label, std::function<void()> callback = nullptr, const std::string &variant = "secondary",
                       bool closes = true);

    protected:
        // Children: our child list owns them
        Div    *_panel{nullptr};
        Div    *_header{nullptr};
        Icon   *_icon{nullptr};
        Label  *_title{nullptr};
        Button *_close{nullptr};
        Div    *_body{nullptr};
        Text   *_description{nullptr};
        Div    *_footer{nullptr};

        std::weak_ptr<Div> _initialFocus{};
        bool               _openedVisible{false}; // The focus was visible when it opened (from the keyboard)

        void updateIcon(std::string_view icon);
        void updateDescription(const std::string &description);
        void opened() override;
        void laidOut() override;
    };
}
