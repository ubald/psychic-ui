#pragma once

#include <string>
#include "psychic-ui/Component.hpp"
#include "psychic-ui/components/Text.hpp"

namespace psychic_ui {
    class TextInput;

    class TextInputSkin : public Skin<TextInput> {
    public:
        TextInputSkin() : Skin<TextInput>() {
            setTag("TextInputSkin");
        }

        virtual void setText(const std::string &/*text*/) {};
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

        std::shared_ptr<Text> textDisplay();
        /**
         * The text changed, with the new text
         */
        Signal<std::string> changed{};
        PSYCHIC_UI_EVENT(onChanged, changed)

    protected:
        void updateText(const std::string &text);

        std::string           _text{};
        std::shared_ptr<Text> _textDisplay{nullptr};
    };
}
