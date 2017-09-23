#include <unicode/unistr.h>
#include "TextInput.hpp"

namespace psychic_ui {

    TextInput::TextInput(const std::string &text) :
        Component<TextInputSkin>(),
        _text(text) {
        setTag("TextInput");
        setWidth(200);
        style()->set(height, 48);
        _focusEnabled = true;
    }


    const std::string &TextInput::getText() const {
        return _text;
    }

    TextInput *TextInput::setText(const std::string &text) {
        if (_text != text) {
            _text = text;
        }
        return this;
    }

    bool TextInput::keyboardCharacterEvent(unsigned int codepoint) {
        icu::UnicodeString uni_str((UChar32)codepoint);
        std::string str;
        uni_str.toUTF8String(str);
        std::cout << str << std::endl;
        return true;
    }
}
