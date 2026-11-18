#include "TextInput.hpp"

namespace psychic_ui {

    TextInput::TextInput(const std::string &text) :
        TextField<TextInputSkin>(text, false) {
        tag("TextInput");
    }

    const std::string &TextInput::placeholder() const {
        return _placeholder;
    }

    void TextInput::updatePlaceholder(const std::string &placeholder) {
        _placeholder = placeholder;
        textChanged();
    }

    void TextInput::textChanged() {
        if (_skin) {
            _skin->setPlaceholder(_placeholder, _editing.text().isEmpty() && !_placeholder.empty());
        }
    }

    void TextInput::textEdited() {
        changed.emit(text());
    }

}
