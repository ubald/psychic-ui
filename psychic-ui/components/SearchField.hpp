#pragma once

#include <memory>
#include <string>
#include "psychic-ui/psychic-ui.hpp"
#include "psychic-ui/Component.hpp"
#include "psychic-ui/Skin.hpp"
#include "psychic-ui/signals/Signal.hpp"
#include "TextInput.hpp"

namespace psychic_ui {
    class SearchField;

    class SearchFieldSkin : public Skin<SearchField> {
    public:
        SearchFieldSkin() : Skin<SearchField>() {
            tag("SearchFieldSkin");
        }
    };

    /**
     * @class SearchField
     *
     * A text field for a query: a search icon, the text (a TextInput, with its placeholder, "Search" by default),
     * and a clear button while it isn't empty. Escape clears it, Enter submits it.
     * The field owns its input and clear button (the `.input` and `.clear` parts, which its skin shows), so a new
     * skin keeps the text.
     */
    class SearchField : public Component<SearchFieldSkin> {
    public:
        explicit SearchField(const std::string &text = "");

        const std::string &text() const;

        template<class Self>
        Self &text(this Self &self, const std::string &text) {
            static_cast<SearchField &>(self).updateText(text);
            return self;
        }

        const std::string &placeholder() const;

        template<class Self>
        Self &placeholder(this Self &self, const std::string &placeholder) {
            static_cast<SearchField &>(self)._input->placeholder(placeholder);
            return self;
        }

        /**
         * The `.input` part: the text input, borderless inside the field
         */
        std::shared_ptr<TextInput> input();

        /**
         * The `.clear` part: clears the field when clicked, hidden while it is empty
         */
        std::shared_ptr<Div> clearButton();

        /**
         * The input's text: Tab, or a press anywhere on the field, lets one type
         */
        Div *focusDelegate() override;

        /**
         * The text changed (typed, set or cleared), with the new text
         */
        Signal<std::string> changed{};
        PSYCHIC_UI_EVENT(onChange, changed)

        /**
         * Enter was pressed, with the text
         */
        Signal<std::string> submitted{};
        PSYCHIC_UI_EVENT(onSubmit, submitted)

        /**
         * While it has text: Escape clears it before closing a popup
         */
        bool takesEscape() const override;

    protected:
        std::shared_ptr<TextInput> _input{nullptr};
        std::shared_ptr<Div>       _clear{nullptr};

        void updateText(const std::string &text);
        void textChanged();
    };
}
