#include <unicode/unistr.h>
#include "TextInput.hpp"

namespace psychic_ui {

    TextInput::TextInput(const std::string &text) :
        Component<TextInputSkin>(),
        _text(text),
        _textDisplay(std::make_shared<Text>(_text)) {
        setTag("TextInput");

        _focusEnabled = true;

        _textDisplay->editable(true);
        _textDisplay->multiline(false);

        _textDisplay->onChange(
            [this](std::string txt) {
                _text = txt;
                changed.emit(_text);
            }
        );
    }

    std::shared_ptr<Text> TextInput::textDisplay() {
        return _textDisplay;
    };

    const std::string &TextInput::text() const {
        return _text;
    }

    void TextInput::updateText(const std::string &text) {
        if (_text != text) {
            _text = text;
            _textDisplay->text(_text);
        }
    }

}
