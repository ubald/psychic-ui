#include <unicode/unistr.h>
#include "TextInput.hpp"

namespace psychic_ui {

    TextInput::TextInput(const std::string &text) :
        Component<TextInputSkin>(),
        _text(text),
        _textDisplay(std::make_shared<Text>(_text)) {
        tag("TextInput");

        _focusEnabled = true;

        _textDisplay->editable(true);
        _textDisplay->multiline(false);

        _textDisplay->onChange(
            [this](std::string txt) {
                _text = txt;
                if (_skin) {
                    _skin->setText(_text);
                }
                changed.emit(_text);
            }
        );
    }

    std::shared_ptr<Text> TextInput::textDisplay() {
        return _textDisplay;
    };

    Div *TextInput::focusDelegate() {
        return _textDisplay.get();
    }

    const std::string &TextInput::text() const {
        return _text;
    }

    void TextInput::updateText(const std::string &text) {
        if (_text != text) {
            _text = text;
            _textDisplay->text(_text);
            if (_skin) {
                _skin->setText(_text);
            }
        }
    }

    const std::string &TextInput::placeholder() const {
        return _placeholder;
    }

    void TextInput::updatePlaceholder(const std::string &placeholder) {
        _placeholder = placeholder;
        if (_skin) {
            _skin->setPlaceholder(_placeholder);
        }
    }

    void TextInput::skinChanged() {
        if (_skin) {
            _skin->setPlaceholder(_placeholder);
            _skin->setText(_text);
        }
    }

}
