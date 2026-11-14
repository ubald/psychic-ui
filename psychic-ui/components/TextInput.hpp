#pragma once

#include <string>
#include "psychic-ui/Component.hpp"
#include "psychic-ui/components/Text.hpp"

namespace psychic_ui {
    class TextInput;

    class TextInputSkin : public Skin<TextInput> {
    public:
        TextInputSkin() : Skin<TextInput>() {
            tag("TextInputSkin");
        }

        virtual void setText(const std::string &/*text*/) {};

        /**
         * What the field shows while it is empty
         */
        virtual void setPlaceholder(const std::string &/*placeholder*/) {};
    };

    class TextInput : public Component<TextInputSkin> {
    public:
        explicit TextInput(const std::string &text = "");
        const std::string &text() const;

        template<class Self>
        Self &text(this Self &self, const std::string &text) {
            static_cast<TextInput &>(self).updateText(text);
            return self;
        }

        /**
         * What the field shows while it is empty (the skin's `.placeholder` part); none by default
         */
        const std::string &placeholder() const;

        template<class Self>
        Self &placeholder(this Self &self, const std::string &placeholder) {
            static_cast<TextInput &>(self).updatePlaceholder(placeholder);
            return self;
        }

        std::shared_ptr<Text> textDisplay();

        /**
         * The text display: Tab, or a press beside the text, lets one type
         */
        Div *focusDelegate() override;

        /**
         * The text changed, with the new text
         */
        Signal<std::string> changed{};
        PSYCHIC_UI_EVENT(onChange, changed)

    protected:
        void updateText(const std::string &text);
        void updatePlaceholder(const std::string &placeholder);
        void skinChanged() override;

        std::string           _text{};
        std::string           _placeholder{};
        std::shared_ptr<Text> _textDisplay{nullptr};
    };
}
