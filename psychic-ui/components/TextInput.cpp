#include <unicode/unistr.h>
#include "TextInput.hpp"

namespace psychic_ui {

    TextInput::TextInput(const std::string &text) :
        Component<TextInputSkin>(),
        _text(text) {
        setTag("TextInput");
        _focusEnabled = true;
    }

    const std::string &TextInput::getText() const {
        return _text;
    }

    TextInput *TextInput::setText(const std::string &text) {
        if (_text != text) {
            _text = text;
            _skin->textDisplay->setText(_text);
        }
        return this;
    }

    void TextInput::skinChanged() {
        Component<TextInputSkin>::skinChanged();

        _skin->textDisplay->setEditable(true);
        _skin->textDisplay->setText(_text);

        if (_onChange) {
            _onChange->disconnect();
            _onChange = nullptr;
        }

        _onChange = _skin->textDisplay->onChange(
            [this](std::string text) {
                _text = text;
                onChange(_text);
            }
        );
    }
}
