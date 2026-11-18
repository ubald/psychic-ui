#include <string>
#include "SearchField.hpp"
#include "TextField.hpp"
#include "psychic-ui/psychic-ui.hpp"

namespace psychic_ui {

    SearchField::SearchField(const std::string &text) :
        TextField<SearchFieldSkin>(text, false) {
        tag("SearchField");
    }

    const std::string &SearchField::placeholder() const {
        return _placeholder;
    }

    void SearchField::updatePlaceholder(const std::string &placeholder) {
        _placeholder = placeholder;
        textChanged();
    }

    void SearchField::clear() {
        if (editable()) {
            apply(_editing.setText(icu::UnicodeString{}));
        }
    }

    bool SearchField::fieldKey(const Key key, const Mod /*mod*/) {
        switch (key) {
            case Key::ESCAPE:
                if (!takesEscape()) {
                    return false;
                }
                clear();
                return true;
            case Key::ENTER:
                submitted.emit(text());
                return true;
            default:
                return false;
        }
    }

    bool SearchField::fieldPress(const MouseEvent &event) {
        if (!_skin) {
            return false;
        }
        int x = 0;
        int y = 0;
        _skin->globalToLocal(x, y, static_cast<int>(event.windowX), static_cast<int>(event.windowY));
        if (!_skin->clearAt(static_cast<float>(x), static_cast<float>(y))) {
            return false;
        }
        clear();
        return true;
    }

    void SearchField::textChanged() {
        if (_skin) {
            const bool empty = _editing.text().isEmpty();
            _skin->setPlaceholder(_placeholder, empty && !_placeholder.empty());
            _skin->setClearable(!empty);
        }
    }

    void SearchField::textEdited() {
        changed.emit(text());
    }

    bool SearchField::takesEscape() const {
        return editable() && !_editing.text().isEmpty();
    }
}
