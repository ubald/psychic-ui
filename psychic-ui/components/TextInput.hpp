#pragma once

#include <string>
#include "psychic-ui/Component.hpp"
#include "psychic-ui/components/Text.hpp"
#include "psychic-ui/components/TextField.hpp"

namespace psychic_ui {
    class TextInput;

    class TextInputSkin : public Skin<TextInput> {
    public:
        TextInputSkin() : Skin<TextInput>() {
            tag("TextInputSkin");
        }

        /**
         * The `.text` part, which the skin creates and owns: the view showing the text, the selection and the caret
         */
        virtual Text *textView() = 0;

        /**
         * What the field shows while it is empty (the `.placeholder` part), and whether it shows now
         */
        virtual void setPlaceholder(const std::string &/*placeholder*/, bool /*shown*/) {};
    };

    /**
     * @class TextInput
     *
     * A text field on one line, with a placeholder while it is empty. Its skin's `"bare"` variant has no chrome, for
     * read-only text one can select and copy (`readOnly(true)`).
     */
    class TextInput : public TextField<TextInputSkin> {
    public:
        explicit TextInput(const std::string &text = "");

        /**
         * What the field shows while it is empty (the skin's `.placeholder` part); none by default
         */
        const std::string &placeholder() const;

        template<class Self>
        Self &placeholder(this Self &self, const std::string &placeholder) {
            static_cast<TextInput &>(self).updatePlaceholder(placeholder);
            return self;
        }

        /**
         * The text changed by an edit (typed, erased, cut or pasted), with the new text
         */
        Signal<std::string> changed{};
        PSYCHIC_UI_EVENT(onChange, changed)

    protected:
        std::string _placeholder{};

        void updatePlaceholder(const std::string &placeholder);
        void textChanged() override;
        void textEdited() override;
    };
}
