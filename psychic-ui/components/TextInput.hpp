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
        const std::string &getText() const;
        TextInput *setText(const std::string &text);
        std::shared_ptr<Text> textDisplay();
        Signal<std::string> onChange{};

    protected:
        std::string           _text{};
        std::shared_ptr<Text> _textDisplay{nullptr};
    };
}
