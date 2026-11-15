#include "SearchField.hpp"
#include "Icon.hpp"

namespace psychic_ui {

    SearchField::SearchField(const std::string &text) :
        Component(),
        _input(std::make_shared<TextInput>(text)),
        _clear(std::make_shared<Div>()) {
        tag("SearchField");

        _focusEnabled = true; // The field takes the focus and hands it to its text

        _input->addClassName("input")
              .placeholder("Search")
              .focusEnabled(false); // Tab stops once, on the field
        _input->onChange([this](const std::string &/*text*/) { textChanged(); });

        // Escape and Enter reach the text display first, which leaves them alone on one line
        _input->textDisplay()->onKeyPress(
            [this](const Key key, const Mod /*modifiers*/) {
                if (key == Key::ESCAPE && !_input->text().empty()) {
                    updateText("");
                } else if (key == Key::ENTER) {
                    submitted.emit(_input->text());
                }
            }
        );

        _clear->addClassName("clear")
              .cursor(Cursor::Arrow)
              .visible(!text.empty());
        _clear->add<Icon>("x").mouseEnabled(false);
        _clear->onClick([this]() { updateText(""); });
    }

    const std::string &SearchField::text() const {
        return _input->text();
    }

    const std::string &SearchField::placeholder() const {
        return _input->placeholder();
    }

    std::shared_ptr<TextInput> SearchField::input() {
        return _input;
    }

    std::shared_ptr<Div> SearchField::clearButton() {
        return _clear;
    }

    Div *SearchField::focusDelegate() {
        return _input->textDisplay().get();
    }

    void SearchField::updateText(const std::string &text) {
        if (text != _input->text()) {
            _input->text(text);
            textChanged(); // Whether or not the text display reported it
        }
    }

    void SearchField::textChanged() {
        const std::string &text = _input->text();
        _clear->visible(!text.empty());
        changed.emit(text);
    }

    bool SearchField::takesEscape() const {
        return _enabled && !_input->text().empty();
    }
}
