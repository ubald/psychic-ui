#pragma once

#include <string>
#include "psychic-ui/psychic-ui.hpp"
#include "psychic-ui/Component.hpp"
#include "psychic-ui/Skin.hpp"
#include "psychic-ui/signals/Signal.hpp"
#include "Text.hpp"
#include "TextField.hpp"

namespace psychic_ui {
    class SearchField;

    class SearchFieldSkin : public Skin<SearchField> {
    public:
        SearchFieldSkin() : Skin<SearchField>() {
            tag("SearchFieldSkin");
        }

        /**
         * The `.text` part, which the skin creates and owns: the view showing the text, the selection and the caret
         */
        virtual Text *textView() = 0;

        /**
         * What the field shows while it is empty (the `.placeholder` part), and whether it shows now
         */
        virtual void setPlaceholder(const std::string &/*placeholder*/, bool /*shown*/) {}

        /**
         * Whether the field has text to clear: shows the `.clear` part
         */
        virtual void setClearable(bool /*clearable*/) {}

        /**
         * Whether a press at this point (in the skin's coordinates) clears the field
         */
        virtual bool clearAt(float x, float y) const = 0;
    };

    /**
     * @class SearchField
     *
     * A text field for a query: its skin shows a search icon, the text, with a placeholder while it is empty
     * ("Search" by default), and a clear button while it has text. Escape clears it, Enter submits it.
     */
    class SearchField : public TextField<SearchFieldSkin> {
    public:
        explicit SearchField(const std::string &text = "");

        /**
         * What the field shows while it is empty (the skin's `.placeholder` part); "Search" by default
         */
        const std::string &placeholder() const;

        template<class Self>
        Self &placeholder(this Self &self, const std::string &placeholder) {
            static_cast<SearchField &>(self).updatePlaceholder(placeholder);
            return self;
        }

        /**
         * The text changed by an edit (typed, erased, cut, pasted or cleared), with the new text
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
        std::string _placeholder{"Search"};

        void updatePlaceholder(const std::string &placeholder);

        /**
         * Empty the field, an edit (`changed` follows)
         */
        void clear();

        bool fieldKey(Key key, Mod mod) override;
        bool fieldPress(const MouseEvent &event) override;
        void textChanged() override;
        void textEdited() override;
    };
}
