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

        onKeyDown([this](Key key) { handleKey(key); });
        onKeyRepeat([this](Key key) { handleKey(key); });

        // Add text display on top of the skin
        _textDisplay = add<Text>(_text);
    }


    const std::string &TextInput::getText() const {
        return _text;
    }

    TextInput *TextInput::setText(const std::string &text) {
        if (_text != text) {
            _text = text;
            _textDisplay->setText(_text);
            updateDisplay();
        }
        return this;
    }

    void TextInput::updateDisplay() {
        _textDisplay->reflow();
    }

    void TextInput::handleKey(Key key) {
        switch (key) {
            case Key::LEFT:
                if (_caretPos > 0) {
                    --_caretPos;
                }
                break;

            case Key::RIGHT:
                if (_caretPos < _text.size()) {
                    ++_caretPos;
                }
                break;

            case Key::BACKSPACE:
                if (_caretPos > 0) {
                    _text.erase(_caretPos - 1, 1);
                    --_caretPos;
                }
                break;

            case Key::DELETE:
                if (_caretPos < _text.size()) {
                    _text.erase(_caretPos, 1);
                    updateDisplay();
                }
                break;

            default:
                break;
        }
    }

    bool TextInput::keyboardCharacterEvent(unsigned int codepoint) {
        icu::UnicodeString uni_str(static_cast<UChar32>(codepoint));
        std::string        str;
        uni_str.toUTF8String(str);
        _text.insert(_caretPos, str);
        ++_caretPos;

        std::cout << _text << std::endl;

        return true;
    }
}
