#pragma once

#include <string>
#include "psychic-ui/Component.hpp"

namespace psychic_ui {
    class TextInput;

    class TextInputSkin : public Skin<TextInput> {
    public:
        TextInputSkin() : Skin<TextInput>() {
            setTag("TextInputSkin");
        }

        virtual void setText(const std::string &text) {};
    };

    class TextInput : public Component<TextInputSkin> {
    public:
        //TextInput() = delete;
        explicit TextInput(const std::string &text = "");

        const std::string &getText() const;
        TextInput *setText(const std::string &text);
    protected:
        std::string _text{};
        unsigned int _caretPos{0};
        void handleKey(Key key);
        bool keyboardCharacterEvent(unsigned int codepoint) override;
    };

    class EditableText: public Div {

    };
}
