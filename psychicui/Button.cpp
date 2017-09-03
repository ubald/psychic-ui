#include <iostream>
#include "Button.hpp"

namespace psychicui {
    Button::Button(const std::string &label)
        : Component() {
        setTag("Button");
        setCursor(Cursor::Hand);

        _label = add<Label>(label);
    }

    Button::Button(const std::string &label, std::function<void()> onClick) :
        Button(label) {
        _onClick = onClick;
    }

    Button::Button(std::function<void()> onClick) :
        Button("", onClick) {

    }

    std::string Button::label() {
        return _label->text();
    }

    void Button::setLabel(std::string label) {
        _label->setText(label);
    }

    void Button::onClick(std::function<void()> callback) {
        _onClick = callback;
    }

    bool Button::selected() {
        return _selected;
    }

    void Button::setSelected(bool selected) {
        _selected = selected;
        invalidateStyle();
    }

    const bool Button::active() const {
        return Component::active() || _selected;
    };

}