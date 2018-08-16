#include <unicode/unistr.h>
#include "TextInput.hpp"

namespace psychic_ui {

    TextInput::TextInput(const std::string &text) :
        Component<TextInputSkin>(),
        _text(text),
        _textDisplay(std::make_shared<Text>(_text)) {
        setTag("TextInput");

        _focusEnabled = true;

        _textDisplay->setEditable(true);
        _textDisplay->setMultiline(false);

        _textDisplay->onChange(
            [this](std::string txt) {
                _text = txt;
                onChange(_text);
            }
        );
    }

    std::shared_ptr<Text> TextInput::textDisplay() {
        return _textDisplay;
    };

    const std::string &TextInput::getText() const {
        return _text;
    }

    TextInput *TextInput::setText(const std::string &text) {
        if (_text != text) {
            _text = text;
            _textDisplay->setText(_text);
        }
        return this;
    }

}
